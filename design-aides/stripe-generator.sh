#!/usr/bin/env bash

set -e

name=$(basename ${0})

TOP_DIR=${TOP_DIR:="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"}

usage () {
	local old_xtrace="$(shopt -po xtrace)"
	set +o xtrace
	echo "${name} - Generates Inkscape sodipodi guidelines for Hannah stripes." >&2
	echo "Usage: ${name} [flags]" >&2
	echo "Option flags:" >&2
	echo "  -a --angle         - Stripe angle in degrees.  Default: ${angle}" >&2
	echo "  -c --count         - Block count.  Default: ${block_count}" >&2
	echo "  -h --help          - Show this help and exit." >&2
	echo "  -v --verbose       - Verbose execution." >&2
	echo "  --block-start      - Width of first block.  Default: ${block_start}" >&2
	echo "  --block-multiplier - Block zoom effect. Default: ${block_multiplier}" >&2
	echo "  --gap-start        - Width of first gap.  Default: ${gap_start}" >&2
	echo "  --gap-multiplier   - Gap zoom effect. Default: ${gap_multiplier}" >&2
	echo "  --gap-ratio        - Block width / Gap Width.  Default: ${gap_ratio}" >&2
	eval "${old_xtrace}"
}

short_opts="a:c:hv"
long_opts="angle:,count:,help,verbose,block-start:,block-multiplier:,gap-start:,gap-multiplier:,gap-ratio:"

opts=$(getopt --options ${short_opts} --long ${long_opts} -n "${name}" -- "$@")

if [ $? != 0 ]; then
	echo "${name}: ERROR: Terminating..." >&2 
	echo "" >&2 
	usage
	exit 1
fi

eval set -- "${opts}"

while true ; do
	case "${1}" in
	-a | --angle)
		angle=${2}
		shift 2
		;;
	-c | --count)
		block_count=${2}
		shift 2
		;;
	-h | --help)
		usage=1
		shift
		;;
	-v | --verbose)
		verbose=1
		shift
		export PS4='\[\033[0;33m\]+$(basename ${BASH_SOURCE}):${LINENO}: \[\033[0;37m\]'
		set -x
		;;
	--block-start)
		block_start=${2}
		shift 2
		;;
	--block-multiplier)
		block_multiplier=${2}
		shift 2
		;;
	--gap-start)
		gap_start=${2}
		shift 2
		;;
	--gap-multiplier)
		gap_multiplier=${2}
		shift 2
		;;
	--gap-ratio)
		gap_ratio=${2}
		shift 2
		;;
	--)
		shift
		break
		;;
	*)
		echo "${name}: ERROR: Internal error: args: '${@}'." >&2
		exit 1
		;;
	esac
done

angle=${angle:=60}
block_count=${block_count:=40}

block_multiplier=${block_multiplier:=0.96}
gap_multiplier=${gap_multiplier:=0.96}

block_start=${block_start:=110}

if [[ ${gap_start} && ${gap_ratio} ]]; then
	echo "${name}: ERROR: Choose --gap-start or --gap-ratio." >&2
	usage
	exit 1
fi

if [[ ${gap_start} ]]; then
	gap_ratio="$(bc <<< "scale=2; ${block_start} / ${gap_start}")"
elif [[ ${gap_ratio} ]]; then
	gap_start="$(bc <<< "scale=2; ${block_start} / ${gap_ratio}")"
else
	gap_start=${gap_start:=35}
	gap_ratio="$(bc <<< "scale=2; ${block_start} / ${gap_start}")"
fi

if [[ ${usage} ]]; then
	usage
	exit 0
fi


on_exit () {
	echo "${name}: Done, ${on_exit_msg}." >&2
}

on_exit_msg="failed"
trap on_exit EXIT HUP INT QUIT PIPE TERM

print_header() {
	echo "gap-ratio=${gap_ratio}" >&2

	echo "<!-- $(date) -->"
	echo "<!-- \
${name} \
--angle=${angle} --count=${block_count} \
--block-start=${block_start} \
--block-multiplier=${block_multiplier} --gap-multiplier=${gap_multiplier} \
--gap-start=${gap_start} --gap-ratio=${gap_ratio} \
-->"
}

print_footer() {
	echo "<!-- ${name} end -->"
}

print_guide() {
	local id=${1}
	local pos=${2}
	local or=${3}
	
	echo "<sodipodi:guide position=\"${pos}\" orientation=\"${or}\" id=\"guide_${id}\" inkscape:locked=\"false\"/>"
}

print_pair_info() {
	echo "${count}: ${block_width}, ${gap_width} => ${gap_pos}, ${block_pos}" >&2
}

print_pair() {
	print_pair_info
	print_guide "gap${count}" "${gap_pos},0" ${orientation}
	print_guide "block${count}" "${block_pos},0" ${orientation}
}

deg_to_rad() {
	local deg=${1}

	bc -l <<< "scale=8; ${deg} * 4 * a(1) / 180"
}

deg_to_or() {
	local deg=${1}
	local rad=$(deg_to_rad ${deg})

	echo "$(bc -l <<< "scale=8; s(${rad})"),$(bc -l <<< "scale=8; -c(${rad})")"
}

orientation="$(deg_to_or ${angle})"

print_header

print_guide horz_0 "0,0" "0,1"
print_guide vert_0 "0,0" "1,0"

count=0
gap_pos=0
gap_width=${gap_start}
block_pos=${gap_start}
block_width=${block_start}
print_pair
let "count = count + 1"

scale=2
while [[ ${count} -lt ${block_count} && \
	$(bc <<< "scale=${scale}; ${block_width} <= 10 || ${gap_width} <= 1") -eq 0 ]]; do
	gap_pos=$(bc <<< "scale=${scale}; ${block_pos} + ${block_width}")
	gap_width=$(bc <<< "scale=${scale}; ${gap_width} * ${gap_multiplier}")
	block_pos=$(bc <<< "scale=${scale}; ${gap_pos} + ${gap_width}")
	block_width=$(bc <<< "scale=${scale}; ${block_width} * ${block_multiplier}")
	print_pair
	let "count = count + 1"
done

print_footer

on_exit_msg="success"
