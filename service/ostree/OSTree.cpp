// Copyright (c) 2019 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "ostree/OSTree.h"

#include "util/Logger.h"

OSTree::OSTree(bool writeLock)
    : m_isInitialized(false)
    , m_sysroot(NULL)
{
    setClassName("OSTree");

    g_autoptr(GError) gerror = NULL;
    gboolean sysrootLockAcquired = FALSE;

    m_sysroot = ostree_sysroot_new(NULL);
    if (!m_sysroot) {
        Logger::error(getClassName(), "Failed to create OstreeSysroot object");
        return;
    }
    if (writeLock) {
        if (!ostree_sysroot_try_lock(m_sysroot, &sysrootLockAcquired, &gerror)) {
            Logger::error(getClassName(), "Failed to lock sysroot: " + string(gerror->message));
            return;
        }
        if (!sysrootLockAcquired) {
            Logger::error(getClassName(), "Failed to lock sysroot");
            return;
        }
    }
    if (!ostree_sysroot_load(m_sysroot, NULL, &gerror)) {
        Logger::error(getClassName(), "Failed to load sysroot: " + string(gerror->message));
        return;
    }

    m_isInitialized = true;
}

OSTree::~OSTree()
{
    if (m_sysroot) {
        ostree_sysroot_unload(m_sysroot);
        g_object_unref(m_sysroot);
    }
}

bool OSTree::deployDelta(const string& path)
{
    OSTree ot(true);
    if (!ot.m_isInitialized)
        return false;

    // extract revision from filename : ostree-630d1ec5-fc6a8911.delta
    // TODO if we know the file format, it will be able to extract the revision.
    string filename = path.substr(0, path.find_last_of("."));
    string toRevision = filename.substr(filename.find_last_of("-") + 1);

    OstreeSysroot* sysroot = ot.m_sysroot;
    g_autoptr(OstreeRepo) repo = NULL;
    g_autoptr(GError) gerror = NULL;
    g_autoptr(GFile) deltaPath = g_file_new_for_path(path.c_str());
    g_autoptr(GKeyFile) originToDeploy = NULL;
    g_autofree char *revisionToDeploy = NULL;
    OstreeDeployment* bootedDeployment = NULL;
    string osname;
    g_autoptr(OstreeDeployment) mergeDeployment = NULL;
    g_autoptr(OstreeDeployment) newDeployment = NULL;
    OstreeSysrootSimpleWriteDeploymentFlags deployFlags = (OstreeSysrootSimpleWriteDeploymentFlags)(OSTREE_SYSROOT_SIMPLE_WRITE_DEPLOYMENT_FLAGS_RETAIN_ROLLBACK);

    // Patch delta
    if (!ostree_sysroot_get_repo(sysroot, &repo, NULL, &gerror)) {
        Logger::error(ot.getClassName(), "Failed to get OstreeRepo object: " + string(gerror->message));
        return false;
    }
    if (!ostree_repo_is_writable(repo, &gerror)) {
        Logger::error(ot.getClassName(), "OstreeRepo is not writable: " + string(gerror->message));
        return false;
    }
    if (!ostree_repo_prepare_transaction(repo, NULL, NULL, &gerror)) {
        Logger::error(ot.getClassName(), "Failed to prepare transaction: " + string(gerror->message));
        return false;
    }
    if (!ostree_repo_static_delta_execute_offline(repo, deltaPath, FALSE, NULL, &gerror)) {
        Logger::error(ot.getClassName(), "Failed to apply delta: " + string(gerror->message));
        return false;
    }
    if (!ostree_repo_commit_transaction(repo, NULL, NULL, &gerror)) {
        Logger::error(ot.getClassName(), "Failed to commit transaction: " + string(gerror->message));
        return false;
    }

    // Deploy
    originToDeploy = ostree_sysroot_origin_new_from_refspec(sysroot, toRevision.c_str());
    if (!ostree_repo_resolve_rev(repo, toRevision.c_str(), FALSE, &revisionToDeploy, &gerror)) {
        Logger::error(ot.getClassName(), "Failed to resolve revision given refspec: " + toRevision + ": " + string(gerror->message));
        return false;
    }
    bootedDeployment = ostree_sysroot_get_booted_deployment(sysroot);
    osname = ostree_deployment_get_osname(bootedDeployment);
    mergeDeployment = ostree_sysroot_get_merge_deployment(sysroot, osname.c_str());
    if (!ostree_sysroot_prepare_cleanup(sysroot, NULL, &gerror)) {
        Logger::error(ot.getClassName(), "Failed to prepare cleanup: " + string(gerror->message));
        return false;
    }
    if (!ostree_sysroot_deploy_tree(sysroot, osname.c_str(), revisionToDeploy, originToDeploy, mergeDeployment, NULL, &newDeployment, NULL, &gerror)) {
        Logger::error(ot.getClassName(), "Failed to deploy tree: " + string(gerror->message));
        return false;
    }
    if (!ostree_sysroot_simple_write_deployment(sysroot, osname.c_str(), newDeployment, mergeDeployment, deployFlags, NULL, &gerror)) {
        Logger::error(ot.getClassName(), "Failed to simple write deployment: " + string(gerror->message));
        return false;
    }
    if (!ostree_sysroot_cleanup(sysroot, NULL, &gerror)) {
        Logger::error(ot.getClassName(), "Failed to cleanup: " + string(gerror->message));
        return false;
    }

    return true;
}

void OSTree::printDebug()
{
    OSTree ot(false);
    if (!ot.m_isInitialized)
        return;

    OstreeSysroot* sysroot = ot.m_sysroot;
    g_autoptr(GPtrArray) deployments = NULL;
    OstreeDeployment* bootedDeployment = NULL;
    g_autoptr(OstreeDeployment) pendingDeployment = NULL;
    g_autoptr(OstreeDeployment) rollbackDeployment = NULL;

    deployments = ostree_sysroot_get_deployments(sysroot);
    bootedDeployment = ostree_sysroot_get_booted_deployment(sysroot);
    ostree_sysroot_query_deployments_for(sysroot, NULL, &pendingDeployment, &rollbackDeployment);
    if (deployments->len == 0) {
        Logger::error(ot.getClassName(), __FUNCTION__, "No deployments");
        return;
    }
    for (guint i = 0; i < deployments->len; i++) {
        OstreeDeployment *deployment = (OstreeDeployment*)deployments->pdata[i];
        string deploymentStatus;
        if (ostree_deployment_is_staged(deployment)) deploymentStatus = " (staged)";
        else if (deployment == pendingDeployment) deploymentStatus = " (pending)";
        else if (deployment == rollbackDeployment) deploymentStatus = " (rollback)";
        GKeyFile *origin = ostree_deployment_get_origin(deployment);

        fprintf(stdout, " %c %s %s.%d%s\n",
                (deployment == bootedDeployment) ? '*' : ' ',
                ostree_deployment_get_osname(deployment),
                ostree_deployment_get_csum(deployment),
                ostree_deployment_get_deployserial(deployment),
                deploymentStatus.c_str());
        if (!origin) {
            fprintf(stdout, "    origin: none\n");
        } else {
            g_autofree char *origin_refspec = g_key_file_get_string(origin, "origin", "refspec", NULL);
            if (!origin_refspec) {
                fprintf(stdout, "    origin: <unknown origin type>\n");
            } else {
                fprintf(stdout, "    origin refspec: %s\n", origin_refspec);
            }
        }
    }
}
