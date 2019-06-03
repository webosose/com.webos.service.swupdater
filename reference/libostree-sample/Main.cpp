#include <iostream>
#include <glib.h>

#include <ostree-1/ostree.h>

using namespace std;

typedef enum {
    EXIT_STATUS_Success = 1,
    EXIT_STATUS_Timeout,
    EXIT_STATUS_Unknown_error,
    EXIT_STATUS_Invalid_arguments
} EXIT_STATUS;

static const gchar *OPTION_SUMMARY =
"This is a libostree sample application for offline delta update.";

static const gchar *OPTION_DESCRIPTION =
"\nExamples:\n\n"
"Apply delta patch on Ostree Repo (/ostree/repo) and queues new deployment as default upon reboot\n"
"$ libostree-tool --delta delta-609f524c-fc6a8911\n\n";

static gchar* option_delta = NULL;

static GOptionEntry OPTION_ENTRIES[] = {
    {
        "delta", 0, 0,
        G_OPTION_ARG_STRING, &option_delta,
        "The path of delta file", NULL
    },
    {
        NULL
    }
};

int main(int argc, char **argv)
{
    GOptionContext* context = g_option_context_new(NULL);
    EXIT_STATUS processResult = EXIT_STATUS_Success;
    g_autoptr(GError) gerror = NULL;

    // ostree
    g_autoptr(OstreeSysroot) sysroot = NULL;
    g_autoptr(OstreeRepo) repo = NULL;
    g_autoptr(GPtrArray) deployments = NULL;
    OstreeDeployment* bootedDeployment = NULL;
    g_autoptr(OstreeDeployment) pendingDeployment = NULL;
    g_autoptr(OstreeDeployment) rollbackDeployment = NULL;
    gboolean sysrootLockAcquired = FALSE;
    GCancellable* cancellable = NULL;
    g_autoptr(GFile) deltaPath = NULL;
    g_autoptr(GKeyFile) originToDeploy = NULL;
    g_autofree char *revisionToDeploy = NULL;
    gchar** tokens = NULL;
    string toRefspec;
    string osname;
    g_autoptr(OstreeDeployment) mergeDeployment = NULL;
    g_autoptr(OstreeDeployment) newDeployment = NULL;
    OstreeSysrootSimpleWriteDeploymentFlags deployFlags = (OstreeSysrootSimpleWriteDeploymentFlags)(OSTREE_SYSROOT_SIMPLE_WRITE_DEPLOYMENT_FLAGS_NO_CLEAN | OSTREE_SYSROOT_SIMPLE_WRITE_DEPLOYMENT_FLAGS_RETAIN_ROLLBACK);
    // ostree

    g_option_context_add_main_entries(context, OPTION_ENTRIES, NULL);
    g_option_context_set_summary(context, OPTION_SUMMARY);
    g_option_context_set_description(context, OPTION_DESCRIPTION);

    if (!g_option_context_parse(context, &argc, &argv, &gerror)) {
        fprintf(stderr, "Option parsing error : %s\n", gerror->message);
        processResult = EXIT_STATUS_Invalid_arguments;
        goto Exit;
    }

    if (!option_delta) {
        gchar* help = g_option_context_get_help(context, FALSE, NULL);
        cout << help << endl;
        g_free(help);
        goto Exit;
    }

    // extract refspec from delta filename : delta-630d1ec5-fc6a8911
    tokens = g_strsplit(option_delta, "-", -1);
    toRefspec = tokens[g_strv_length(tokens)-1];

    // 0. prepare
    sysroot = ostree_sysroot_new(NULL);
    if (!sysroot) {
        fprintf(stderr, "Failed to create OstreeSysroot object\n");
        goto Exit;
    }
    // Acquire exclusive multi-process write lock
    if (!ostree_sysroot_try_lock(sysroot, &sysrootLockAcquired, &gerror) || !sysrootLockAcquired) {
        fprintf(stderr, "Failed to lock sysroot : %s\n", gerror->message);
        goto Exit;
    }
    if (!ostree_sysroot_load(sysroot, NULL, &gerror)) {
        fprintf(stderr, "Failed to load sysroot : %s\n", gerror->message);
        goto Exit;
    }

    // 1. patch delta to repo
    if (!ostree_sysroot_get_repo(sysroot, &repo, cancellable, &gerror)) {
        fprintf(stderr, "Failed to get OstreeRepo object : %s\n", gerror->message);
        goto Exit;
    }
    if (!ostree_repo_is_writable(repo, &gerror)) {
        fprintf(stderr, "OstreeRepo is not writable : %s\n", gerror->message);
        goto Exit;
    }
    if (!ostree_repo_prepare_transaction(repo, NULL, cancellable, &gerror)) {
        fprintf(stderr, "Failed to prepare transaction : %s\n", gerror->message);
        goto Exit;
    }
    deltaPath = g_file_new_for_path(option_delta);
    if (!ostree_repo_static_delta_execute_offline(repo, deltaPath, FALSE, cancellable, &gerror)) {
        fprintf(stderr, "Failed to apply delta : %s\n", gerror->message);
        goto Exit;
    }
    if (!ostree_repo_commit_transaction(repo, NULL, cancellable, &gerror)) {
        fprintf(stderr, "Failed to commit transaction : %s\n", gerror->message);
        goto Exit;
    }

    // 2. deploy to sysroot
    originToDeploy = ostree_sysroot_origin_new_from_refspec(sysroot, toRefspec.c_str());
    if (!ostree_repo_resolve_rev(repo, toRefspec.c_str(), FALSE, &revisionToDeploy, &gerror)) {
        fprintf(stderr, "Failed to resolve revision given refspec (%s) : %s\n", toRefspec.c_str(), gerror->message);
        goto Exit;
    }
    bootedDeployment = ostree_sysroot_get_booted_deployment(sysroot);
    osname = ostree_deployment_get_osname(bootedDeployment);
    mergeDeployment = ostree_sysroot_get_merge_deployment(sysroot, osname.c_str());
    if (!ostree_sysroot_prepare_cleanup(sysroot, cancellable, &gerror)) {
        fprintf(stderr, "Failed to prepare cleanup : %s\n", gerror->message);
        goto Exit;
    }
    if (!ostree_sysroot_deploy_tree(sysroot, osname.c_str(), revisionToDeploy, originToDeploy, mergeDeployment, NULL, &newDeployment, cancellable, &gerror)) {
        fprintf(stderr, "Failed to deploy tree : %s\n", gerror->message);
        goto Exit;
    }
    if (!ostree_sysroot_simple_write_deployment(sysroot, osname.c_str(), newDeployment, mergeDeployment, deployFlags, cancellable, &gerror)) {
        fprintf(stderr, "Failed to simple write deployment : %s\n", gerror->message);
        goto Exit;
    }
    if (!ostree_sysroot_cleanup(sysroot, cancellable, &gerror)) {
        fprintf(stderr, "Failed to cleanup : %s\n", gerror->message);
        goto Exit;
    }

    // delete more than 3 deployments
    deployments = ostree_sysroot_get_deployments(sysroot);
    for (guint i = deployments->len - 1; i >= 3; i--) {
        g_ptr_array_remove_index(deployments, i);
    }
    if (!ostree_sysroot_write_deployments(sysroot, deployments, cancellable, &gerror)) {
        fprintf(stderr, "Failed to write deployments : %s\n", gerror->message);
        goto Exit;
    }

    // 3. debug (ostree admn status)
    deployments = ostree_sysroot_get_deployments(sysroot);
    bootedDeployment = ostree_sysroot_get_booted_deployment(sysroot);
    ostree_sysroot_query_deployments_for(sysroot, NULL, &pendingDeployment, &rollbackDeployment);
    if (deployments->len == 0) {
        fprintf(stderr, "No deployments.\n");
        goto Exit;
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

Exit:
    if (tokens) {
        g_strfreev(tokens);
    }
    if (option_delta) {
        g_free(option_delta);
    }
    if (context) {
        g_option_context_free(context);
    }
    return processResult;
}
