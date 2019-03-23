#!/usr/bin/env bash

set -e

name=$(basename ${0})

TOP_DIR=${TOP_DIR:="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"}
FDL=${FDL:="${TOP_DIR}/fabricators-design-license.txt"}
TEMPLATE=${TEMPLATE:="${TOP_DIR}/work-template.rdf"}
AUTHOR=${AUTHOR:="$(git config --get user.name) \&lt\;$(git config --get user.email)\&gt\;"}

usage () {
	echo "${name} - Clean svg metadata." >&2
	echo "Usage: ${name} [flags] <target> <target> ..." >&2
	echo "Option flags:" >&2
	echo "  -a --author <author> - author/creator/owner of work. Default: '${AUTHOR}'." >&2
	echo "  -c --clean           - Remove metadata." >&2
	echo "  -h --help            - Show this help and exit." >&2
	echo "  -k --keep            - Keep tmp files on exit." >&2
	echo "  -l --licence         - Replace template licence and copyright entries." >&2
	echo "  -t --template        - Replace metadata with template." >&2
	echo "  -v --verbose         - Verbose execution." >&2
	echo "Environment:" >&2
	echo "  FDL                  - Default: '${FDL}'" >&2
	echo "  TEMPLATE             - Default: '${TEMPLATE}'" >&2
	echo "  AUTHOR               - Default: '${AUTHOR}'" >&2
	echo "Examples:" >&2
	echo "  ${name} -c." >&2
	echo "  ${name} -tl." >&2
}

short_opts="a:chkltv"
long_opts="author:,clean,help,keep,licence,template,verbose"

opts=$(getopt --options ${short_opts} --long ${long_opts} -n "${name}" -- "$@")

if [ $? != 0 ]; then
	echo "${name}: ERROR: Terminating..." >&2 
	echo "" >&2 
	usage
	exit 1
fi

eval set -- "${opts}"

if [[ "${1}" == "--" ]]; then
	echo "${name}: ERROR: No options specified." >&2
	echo "" >&2 
	usage
	exit 1
fi

while true ; do
	case "${1}" in
	-a | --author)
		AUTHOR="${2}"
		shift 2
		;;
	-c | --clean)
		clean=1
		shift
		;;
	-h | --help)
		usage=1
		shift
		;;
	-k | --keep)
		keep_tmp=1
		shift
		;;
	-l | --licence)
		licence=1
		shift
		;;
	-t | --template)
		template=1
		shift
		;;
	-v | --verbose)
		verbose=1
		shift
		;;
	--)
		shift
		while [[ -n ${1} ]]; do
			targets="${targets} ${1}"
			shift
		done
		break
		;;
	*)
		echo "${name}: ERROR: Internal error: args: '${@}'." >&2
		exit 1
		;;
	esac
done

if [[ -n ${usage} ]]; then
	usage
	exit 0
fi

progs="xmlstarlet git"

for p in ${progs}; do
	if ! command -v "${p}" > /dev/null; then
		echo "Please install ${p}"
		result=1
	fi
done

[ -z ${result} ] || exit 1

if [[ -z ${targets} ]]; then
	echo "${name}: ERROR: No targets specified." >&2
	echo "" >&2
	usage
	exit 1
fi

if [[ -n ${template} && ! -f ${TEMPLATE} ]]; then
	echo "${name}: ERROR: Bad TEMPLATE: ${TEMPLATE}." >&2
	echo "" >&2
	usage
	exit 1
fi

if [[ -n ${licence} && ! -f ${FDL} ]]; then
	echo "${name}: ERROR: Bad FDL: ${FDL}." >&2
	echo "" >&2
	usage
	exit 1
fi

if [[ -n ${verbose} ]]; then
	set -x
fi

tmp_dir=$(mktemp --tmpdir --directory  mx-graphics.XXXX)

on_exit () {
	[[ -n ${keep_tmp} ]] || rm -rf ${tmp_dir}
	
	echo "${name}: Done, ${on_exit_msg}." >&2
}

on_exit_msg="failed"
trap on_exit EXIT HUP INT QUIT PIPE TERM

token="\@\@work\@\@"

process_targets () {
	local t

	for t in ${targets}; do
		if [[ -d ${t} ]]; then
			files="${files} $(find "${t}" -name '*.svg')"
		elif [[ -f ${t} ]]; then
			files="${files} ${t}"
		else
			echo "${name}: ERROR: Bad target: ${t}" >&2
			echo "" >&2
			usage
			exit 1
		fi
	done
}

validate_files () {
	for f in ${files}; do
		if ! xmlstarlet --quiet validate ${f}; then
			echo "${name}: ERROR: Invalid XML: ${f}" >&2
			result=1
		fi
	done

	if [[ -n ${result} ]]; then
		echo "${name}: ERROR: validate_files failed." >&2
		exit 1
	fi
}

check_files () {
	local result
	local f

	for f in ${files}; do
		if [[ ! -f ${f} ]]; then
			echo "${name}: ERROR: Not file target: ${f}" >&2
			result=1
			continue
		fi
	done

	if [[ -n ${result} ]]; then
		echo "${name}: ERROR: check_files failed." >&2
		exit 1
	fi

	validate_files
}

clean () {
	local f=${1}
	local tmp="${tmp_dir}/${f##*/}"

	# -0=whole file, -p=loop, -e=program
	# g=global, m=multiline, s=match newline
	perl -0pe "s/<cc:Work.*\/cc:Work>/${token}/gms" ${f} > ${tmp}.1

	xmlstarlet format --indent-spaces 2 ${tmp}.1 > ${tmp}.2

	cp -H ${tmp}.2 ${f}
	[[ -n ${keep_tmp} ]] || rm -f ${tmp}.1 ${tmp}.2
}

template () {
	local f=${1}
	local tmp="${tmp_dir}/${f##*/}"

	clean ${f}

	sed -e "/${token}/r${TEMPLATE}" -e "/${token}/d" ${f} > ${tmp}.1

	xmlstarlet --quiet validate ${tmp}.1
	xmlstarlet format --indent-spaces 2 ${tmp}.1 > ${tmp}.2

	cp -H ${tmp}.2 ${f}
	[[ -n ${keep_tmp} ]] || rm -f ${tmp}.1 ${tmp}.2
}

licence () {
	local f=${1}
	local tmp="${tmp_dir}/${f##*/}"

	sed -e "s/@@description@@/@@description@@\n        /" ${f} > ${tmp}.1
	sed --in-place -e "/@@description@@/r${FDL}" -e "s/@@description@@//" ${tmp}.1

	sed --in-place \
		-e "s/@@creator@@/${AUTHOR}/" \
		-e "s/@@owner@@/${AUTHOR}/" \
		-e "s/@@rights@@/All Rights Reserved/" \
		-e "s/@@licence@@/Fabricators Design License/" \
		${tmp}.1

	xmlstarlet --quiet validate ${tmp}.1
	xmlstarlet format --indent-spaces 2 ${tmp}.1 > ${tmp}.2

	cp -H ${tmp}.2 ${f}
	[[ -n ${keep_tmp} ]] || rm -f ${tmp}.1 ${tmp}.2
}

process_targets
check_files

for f in ${files}; do
	[[ -z ${clean} ]] || clean ${f}
	[[ -z ${template} ]] || template ${f}
	[[ -z ${licence} ]] || licence ${f}
done

on_exit_msg="success"
