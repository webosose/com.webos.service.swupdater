#!/bin/bash

function showHelp {
    echo "Required tools: "
    echo "  ostree"
    echo "  curl"
    echo "  jq               command line json handler"
    echo
    echo "Prepare local repo: (run this once at first)"
    echo "$0 \\"
    echo "  --prepare        Prepare local repo"
    echo "  --image          http://webosimg.lge.com/images/ostree/{image} (default: webos-master-official-raspberrypi4)"
    echo "  --repo           The path for local repo (default: current dir)"
    echo "  --branch         [webos-image-master | webos-image-devel-master] (default: webos-image-master)"
    echo
    echo "Generate the latest delta from the official server and upload it to the hawkbit server: "
    echo "$0 \\"
    echo "  --image          http://webosimg.lge.com/images/ostree/{image} (default: webos-master-official-raspberrypi4)"
    echo "  --repo           The path for local repo (default: current dir)"
    echo "  --branch         [webos-image-master | webos-image-devel-master] (default: webos-image-master)"
    echo "  --from           From revision (default: The latest previous version)"
    echo "  --to             To revision (default: The latest version)"
    echo "  --addr           hawkBit address (default: http://10.178.84.116:8080)"
    echo "  --user           hawkBit user (default: admin)"
    echo "  --pass           hawkBit pass (default: admin)"
    echo
    echo "Usages:"
    echo "$0 --repo=/home/myungchul/work/ostree_repo_official --prepare"
    echo "$0 --repo=/home/myungchul/work/ostree_repo_official --addr=http://10.177.247.56:8888"
}

for i in "$@"; do
    case $i in
    -h|--help)
        showHelp;
        exit 0;;
    --prepare)
        Prepare="1";;
    --image=*)
        Image="${i#*=}"
        shift;; # past argument=value
    --repo=*)
        Repo="${i#*=}"
        shift;;
    --branch=*)
        Branch="${i#*=}"
        shift;;
    --from=*)
        From="${i#*=}"
        shift;;
    --to=*)
        To="${i#*=}"
        shift;;
    --addr=*)
        Addr="${i#*=}"
        shift;;
    --user=*)
        User="${i#*=}"
        shift;;
    --pass=*)
        Pass="${i#*=}"
        shift;;
    esac
done

Image=${Image:="webos-master-official-raspberrypi4"}
Repo=${Repo:="."}
Branch=${Branch:="webos-image-master"}

if [ "$Prepare" == "1" ]; then
    set -x
    ostree init --repo=${Repo} --mode=archive-z2
    ostree remote add --no-gpg-verify official http://webosimg.lge.com/images/ostree/${Image}
    set +x
    exit 0
fi

From=${From:=`ostree log --repo=$Repo $Branch | grep commit | sed -n 2p | cut -c8-15`}
From=${From:0:8}
To=${To:=`ostree log --repo=$Repo $Branch | grep commit | sed -n 1p | cut -c8-15`}
To=${To:0:8}
Addr=${Addr:="http://10.178.84.116:8080"}
User=${User:="admin"}
Pass=${Pass:="admin"}
Auth="Authorization: Basic `echo -n $User:$Pass | base64`"

echo "Image       = $Image"
echo "Repo        = $Repo"
echo "Branch      = $Branch"
echo "From        = $From"
echo "To          = $To"
echo "Addr        = $Addr"
echo "User        = $User"
echo "Pass        = $Pass"
echo "Auth        = $Auth"

FromVersion=`ostree log --repo=$Repo $Branch | grep "commit\|${Branch}" | sed -n "/^commit $From/{n;p}" | sed -e "s/[[:space:]]*$Branch-//g"`
ToVersion=`ostree log --repo=$Repo $Branch | grep "commit\|${Branch}" | sed -n "/^commit $To/{n;p}" | sed -e "s/[[:space:]]*$Branch-//g"`
Filename=${Repo}/ostree-${From}-${To}.delta

echo "FromVersion = $FromVersion"
echo "ToVersion   = $ToVersion"
echo "Filename    = $Filename"

echo
echo "Pulling ${Branch} from official repo..."
set -x
ostree pull --repo=$Repo official:${Branch} --depth=5
set +x

echo
echo "Generating static delta..."
set -x
ostree static-delta generate --repo=$Repo --min-fallback-size=0 --inline --from=$From --to=$To --filename=$Filename
set +x

echo
echo "Creating softwaremodule..."
set -x
SoftwaremoduleRes=`curl $Addr/rest/v1/softwaremodules -sb -i -X POST -H "${Auth}" -H "Content-Type: application/hal+json;charset=UTF-8" -d "[{\"type\": \"os\", \"name\": \"${Image}\", \"version\": \"${ToVersion}\"}]"`
set +x
Softwaremodule=`echo $SoftwaremoduleRes | jq -r '.[].id'`

echo
echo "Uploading delta..."
set -x
ArtifactRes=`curl $Addr/rest/v1/softwaremodules/$Softwaremodule/artifacts -sb -i -X POST -H "${Auth}" -H "Content-Type: multipart/form-data" -F "file=@$Filename"`
set +x
Artifact=`echo $ArtifactRes | jq -r '.id'`

echo
echo "Creating distribution..."
set -x
DistributionRes=`curl $Addr/rest/v1/distributionsets -sb -i -X POST -H "${Auth}" -H "Content-Type: application/json;charset=UTF-8" -d "[{\"type\": \"os\", \"name\": \"${Image}\", \"version\": \"${ToVersion}\", \"modules\": [{\"id\": ${Softwaremodule}}], \"requiredMigrationStep\": false}]"`
set +x
Distribution=`echo $DistributionRes | jq -r '.[].id'`

echo
echo "Creating filter..."
set -x
FilterRes=`curl $Addr/rest/v1/targetfilters -sb -i -X POST -H "${Auth}" -H "Content-Type: application/json" -d "{\"name\": \"webos_build_id : ${FromVersion}\", \"query\": \"attribute.webos_build_id == ${FromVersion}\"}"`
set +x
Filter=`echo $FilterRes | jq -r '.id'`

echo
echo "Auto-assigning distribution to filter..."
set -x
AutoAssignRes=`curl $Addr/rest/v1/targetfilters/$Filter/autoAssignDS -sb -i -X POST -H "${Auth}" -H "Content-Type: application/json" -d "{\"id\": \"${Distribution}\", \"type\": \"soft\"}"`
set +x

echo
echo "Done"
