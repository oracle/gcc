#!/bin/bash

#Jose's defaults
#Also for those who prefer changing the script.
PRODUCT=gcc
OLBRANCH=el7-u9
GITBASE=cf82a597b0d189857acb34a08725762c4f5afb50
OLSVN=/home/qinzhao/Backport/branches-7/el7-u9/

# Print usage
usage() {
  echo -n "Genolbranch.sh cretes olbranches for corresponding svn repositories.
For details on this process please check the confluence page:
https://confluence.oraclecorp.com/confluence/display/linuxtoolchain/Workflow

Please run this script at the root of the ca-tools4 repo that you would
like to branch.

Here are some command line arguments that you might want to set:
 Options:
  -p, --product     Prossible values are binutils, c, gdb, glibc etc.
  -r, --svnrepo     Relative or absolute path of the svn repo.
                    This is the folder containing the .spec file
  -o, --olbranch    Possible values are el6u10, el7u5 etc.
  -g, --gitbase     The commit hash of the corresponding vanilla source.

Usage:
./genolbranch.sh [-p <productname>] [-r <path>] [-o <version>] [-g <hash>]
"
}

OPTS=`getopt -o p:r:o:g:h --long product:,svnrepo:,olbranch:,gitbase:,help -n 'genolbranch.sh' -- "$@"`

if [ $? != 0 ] ; then echo "Failed parsing options." >&2 ; exit 1 ; fi

echo "$OPTS"
eval set -- "$OPTS"

while true; do
  case "$1" in
    -p | --product ) 	PRODUCT="$2"; 	shift; shift ;;
    -r | --svnrepo )    OLSVN="$2"; 	shift; shift ;;
    -o | --olbranch ) 	OLBRANCH="$2"; 	shift; shift ;;
    -g | --gitbase ) 	GITBASE="$2"; 	shift; shift ;;
    -h | --help ) 	usage;	 	exit ;;
    -- ) 				shift; break ;;
    * ) echo "Unknown argument $1"; 	exit ;;
  esac
done

SPEC=$PRODUCT.spec
GITBRANCH=oracle/$PRODUCT/$OLBRANCH

echo PRODUCT         = "${PRODUCT}"
echo OLBRANCH        = "${OLBRANCH}"
echo GITBASE         = "${GITBASE}"
echo SPEC            = "${SPEC}"
echo GITBRANCH       = "${GITBRANCH}"
echo OLSVN           = "${OLSVN}"


################################################################################

echo "Getting a list of patches to apply..."

pushd $OLSVN

#Get the patch numbers and their names:
NUMBERS_AND_NAMES=$(egrep "^Patch[0-9]+" $SPEC)
NUMBERS=($(echo "$NUMBERS_AND_NAMES" | awk '{print $1}' | grep -o [1-9][0-9]*))
NAMES=($(echo "$NUMBERS_AND_NAMES" | awk '{print $2}'))
declare -A patches
for (( i=0; i<${#NUMBERS[@]} ; i++ )) ; do
    #echo "${NUMBERS[i]}" "${NAMES[i]}"
    patches[${NUMBERS[i]}]=${NAMES[i]}
done
declare -A -p patches

#Get the patch numbers and their depths:
NUMBERS_AND_DEPTHS=$(egrep "^.patch[0-9]+ -p[0-9]" $SPEC -o )
NUMBERS=($(echo "$NUMBERS_AND_DEPTHS" | awk '{print $1}' | grep -o [1-9][0-9]*))
declare -p NUMBERS
DEPTHS=($(echo "$NUMBERS_AND_DEPTHS" | awk '{print $2}'))
declare -p DEPTHS
#Add it to the dictionary:
declare -A depths
for (( i=0; i<${#NUMBERS[@]} ; i++ )) ; do
    echo "${NUMBERS[i]}" "${DEPTHS[i]}"
    depths[${NUMBERS[i]}]=${DEPTHS[i]}
done
declare -A -p depths


################################################################################

svn_rpmbuild () {
    mkdir -p $PWD/rpmbuild/{RPMS,BUILD{,ROOT},SRPMS}
    rpmbuild --define="_topdir $PWD/rpmbuild" \
             --define="_sourcedir $PWD" \
             --define="_specdir $PWD" \
             --define="_tmppath $PWD/rpmbuild/BUILDROOT" \
             $(test -f dist && echo "--define=dist.$(<dist)") \
             "$@"
}

# Execute rpmbuild to make sure we do not apply any patches that might be skip
# by the spec file.
echo $PWD
#ORDERED_PATCH_NUMBERS=($(svn_rpmbuild --nodeps -bp $SPEC 2>&1 | grep -o '^Patch #[0-9]*' | grep -o [0-9]*))
#ORDERED_PATCH_NUMBERS=($(svn_rpmbuild --nodeps -bp $SPEC 2>&1 | grep -o '^Patch #[0-9]*' | grep -o '[0-9]*')) 
#declare -A -p ORDERED_PATCH_NUMBERS

popd

################################################################################

#Apply the patches and commit:
#git checkout $GITBASE
#git branch $GITBRANCH
#git checkout $GITBRANCH

#for i in "${ORDERED_PATCH_NUMBERS[@]}" ; do
for i in "${NUMBERS[@]}" ; do
    echo "$i" "${patches[$i]}" "${depths[$i]}"
    echo "------- APPLYING ${patches[$i]} ---------"
    patch -V never ${depths[$i]} < $OLSVN/${patches[$i]}
    find . -name '*.orig' -delete
    git add --all .
    git commit -a -m "${patches[$i]}"
done
