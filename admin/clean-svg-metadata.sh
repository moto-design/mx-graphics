#!/usr/bin/env bash

usage() {
	local old_xtrace
	old_xtrace="$(shopt -po xtrace || :)"
	set +o xtrace
	echo "${script_name} - Clean svg metadata." >&2
	echo "Usage: ${script_name} [flags] <target> <target> ..." >&2
	echo "Option flags:" >&2
	echo "  -c --check           - Run shellcheck." >&2
	echo "  -h --help            - Show this help and exit." >&2
	echo "  -v --verbose         - Verbose execution." >&2
	echo "  -a --author <author> - author/creator/owner of work. Default: '${author}'." >&2
	echo "  -k --keep            - Keep tmp files on exit." >&2
	echo "Operation flags:" >&2
	echo "  -r --clean           - Remove metadata." >&2
	echo "  -t --template        - Replace metadata with template." >&2
	echo "  -l --license         - Replace template license and copyright entries." >&2
	echo "Environment:" >&2
	echo "  FDL                  - Default: '${FDL}'" >&2
	echo "  TEMPLATE             - Default: '${TEMPLATE}'" >&2
	echo "Examples:" >&2
	echo "  ${script_name} -r." >&2
	echo "  ${script_name} -tl." >&2
	eval "${old_xtrace}"
}

process_opts() {
	local short_opts="chva:krtl"
	local long_opts="check,help,verbose,\
author:,keep,clean,template,license"

	local opts
	opts=$(getopt --options ${short_opts} --long ${long_opts} -n "${script_name}" -- "$@")

	eval set -- "${opts}"

	while true ; do
		# echo "${FUNCNAME[0]}: (${#}) '${*}'"
		case "${1}" in
		-c | --check)
			check=1
			shift
			;;
		-h | --help)
			usage=1
			shift
			;;
		-v | --verbose)
			#verbose=1
			set -x
			shift
			;;
		-a | --author)
			author="${2}"
			shift 2
			;;
		-k | --keep)
			keep_tmp=1
			shift
			;;
		-r | --clean)
			clean=1
			shift
			;;
		-t | --template)
			template=1
			shift
			;;
		-l | --license)
			license=1
			shift
			;;
		--)
			shift
			targets="${*}"
			break
			;;
		*)
			echo "${script_name}: ERROR: Internal opts: '${*}'" >&2
			exit 1
			;;
		esac
	done
}

on_exit() {
	local result=${1}

	if [[ ! ${keep_tmp} && -d "${tmp_dir}" ]]; then
		rm -rf "${tmp_dir}"
	fi

	echo "${script_name}: Done: ${result}." >&2
}

check_file() {
	local src="${1}"
	local msg="${2}"
	local usage="${3}"

	if [[ ! -f "${src}" ]]; then
		echo -e "${script_name}: ERROR: File not found${msg}: '${src}'" >&2
		[[ -z "${usage}" ]] || usage
		exit 1
	fi
}

process_targets() {
	local -n _process_targets__files=${1}
	local t

	for t in ${targets}; do
		if [[ -d ${t} ]]; then
			_process_targets__files+="$(find "${t}" -name '*.svg')"
		elif [[ -f ${t} ]]; then
			_process_targets__files+="${t}"
		else
			echo "${script_name}: ERROR: Bad target: ${t}" >&2
			echo "" >&2
			usage
			exit 1
		fi
	done
}

validate_files() {
	local files="${*}"
	local f

	for f in ${files}; do
		if ! xmlstarlet --quiet validate "${f}"; then
			echo "${script_name}: ERROR: Invalid XML: ${f}" >&2
			result=1
		fi
	done

	if [[ -n ${result} ]]; then
		echo "${script_name}: ERROR: validate_files failed." >&2
		exit 1
	fi
}

check_files() {
	local files="${*}"
	local f
	local result

	for f in ${files}; do
		if [[ ! -f ${f} ]]; then
			echo "${script_name}: ERROR: Not file target: ${f}" >&2
			result=1
			continue
		fi
	done

	if [[ -n ${result} ]]; then
		echo "${script_name}: ERROR: check_files failed." >&2
		exit 1
	fi

	validate_files "${files}"
}

check_svg_rdf() {
	local file=${1}

	if ! grep -E '<rdf:RDF' "${file}" >/dev/null; then
		echo "${script_name}: ERROR: No '<rdf:RDF' tag ." >&2
		exit 1
	fi
}

check_rdf_token() {
	local file=${1}

	if ! grep -E "${rdf_token}" "${file}" >/dev/null; then
		echo "${script_name}: ERROR: No '${rdf_token}' token." >&2
		exit 1
	fi
}

clean() {
	local file=${1}
	local tmp="${tmp_dir}/${file##*/}"

	# -0=whole file, -p=loop, -e=program
	# g=global, m=multiline, s=match newline
	perl -0pe "s/<rdf:RDF.*\/rdf:RDF>/${rdf_token}/gms" "${file}" > "${tmp}.clean1"

	xmlstarlet format --indent-spaces 2 "${tmp}.clean1" > "${tmp}.clean2"

	cp -H "${tmp}.clean2" "${file}"
}

template() {
	local file=${1}
	local tmp="${tmp_dir}/${file##*/}"

	sed -e "/${rdf_token}/r${TEMPLATE}" -e "/${rdf_token}/d" "${file}" > "${tmp}.template1"

	xmlstarlet --quiet validate "${tmp}.template1"
	xmlstarlet format --indent-spaces 2 "${tmp}.template1" > "${tmp}.template2"

	cp -H "${tmp}.template2" "${file}"
}

license() {
	local file=${1}
	local tmp="${tmp_dir}/${file##*/}"

	sed -e "s/@@description@@/@@description@@\n        /" "${file}" > "${tmp}.license1"
	sed --in-place -e "/@@description@@/r${FDL}" -e "s/@@description@@//" "${tmp}.license1"

	sed --in-place \
		-e "s/@@creator@@/${author}/" \
		-e "s/@@owner@@/${author}/" \
		-e "s/@@rights@@/All Rights Reserved/" \
		-e "s/@@license@@/Fabricators Design License/" \
		"${tmp}.license1"

	xmlstarlet --quiet validate "${tmp}.license1"
	xmlstarlet format --indent-spaces 2 "${tmp}.license1" > "${tmp}.license2"

	cp -H "${tmp}.license2" "${file}"
}

#===============================================================================
export PS4='\[\e[0;33m\]+ ${BASH_SOURCE##*/}:${LINENO}:(${FUNCNAME[0]:-main}):\[\e[0m\] '

script_name="${0##*/}"

trap "on_exit 'Failed'" EXIT
set -e

SCRIPTS_TOP=${SCRIPTS_TOP:-"$(cd "${BASH_SOURCE%/*}" && pwd)"}

process_opts "${@}"

FDL=${FDL:="${SCRIPTS_TOP}/fabricators-design-license.txt"}
TEMPLATE=${TEMPLATE:="${SCRIPTS_TOP}/template-min.rdf"}
author=${author:="$(git config --get user.name) \&lt\;$(git config --get user.email)\&gt\;"}

#build_time="$(date +%Y.%m.%d-%H.%M.%S)"
rdf_token="\@\@rdf\@\@"

if [[ ${usage} ]]; then
	usage
	trap - EXIT
	exit 0
fi

if [[ ${check} ]]; then
	shellcheck=${shellcheck:-"shellcheck"}

	if ! test -x "$(command -v "${shellcheck}")"; then
		echo "${script_name}: ERROR: Please install '${shellcheck}'." >&2
		exit 1
	fi

	${shellcheck} "${0}"
	trap "on_exit 'Success'" EXIT
	exit 0
fi

progs="xmlstarlet git"

for p in ${progs}; do
	if ! command -v "${p}" > /dev/null; then
		echo "Please install ${p}"
		result=1
	fi
done

[ ${result} ] && exit 1


if [[ ! ${targets} ]]; then
	echo "${script_name}: ERROR: No targets specified." >&2
	echo "" >&2
	usage
	exit 1
fi


[[ ${template} ]] && check_file "${TEMPLATE}"
[[ ${license} ]] && check_file "${FDL}"

tmp_dir=$(mktemp --tmpdir --directory  mx-graphics.XXXX)

target_files=""

process_targets target_files
check_files ${target_files}



for f in ${target_files}; do
	if [[ ${clean} ]]; then
		check_svg_rdf "${f}"	
		clean "${f}"
	fi
	if [[ ${template} ]]; then
		clean "${f}"
		check_rdf_token "${f}"
		template "${f}"
	fi
	if [[ ${license} ]]; then
		license "${f}"
	fi
done

trap "on_exit 'Success'" EXIT
exit 0

