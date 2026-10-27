#!/bin/sh

set -u

# Use available CPUs by default; TEST_JOBS can override the worker count.
jobs=${TEST_JOBS:-$(nproc)}
case "$jobs" in
	''|*[!0-9]*|0)
		echo "TEST_JOBS must be a positive integer" >&2
		exit 2
		;;
esac

test_list=$(./sample_test --gtest_list_tests) || {
	echo "Could not list sample_test cases" >&2
	exit 2
}

# Keep logs and temporary scheduling data out of the source tree.
output_dir=$(mktemp -d) || {
	echo "Could not create temporary directory for test output" >&2
	exit 2
}
cache_file=.sample_test_timings.tsv
cache_tmp=./.sample_test_timings.tsv.tmp.$$
trap 'rm -rf "$output_dir"; rm -f "$cache_tmp"' EXIT

printf '%s\n' "$test_list" | awk '
	/^[^[:space:]].*\.$/ {
		suite = $1
		sub(/\.$/, "", suite)
		next
	}
	/^  [^#[:space:]]/ { print suite "." $1 }
' >"$output_dir/tests"

# Google Test lists suite names and test names on separate lines.
test_count=$(awk 'END { print NR + 0 }' "$output_dir/tests")
if [ "$test_count" -eq 0 ]; then
	echo "sample_test did not report any tests" >&2
	exit 2
fi

shard_count=$jobs
if [ "$shard_count" -gt "$test_count" ]; then
	shard_count=$test_count
fi

if [ ! -f "$cache_file" ]; then
	: >"$cache_file" || exit 2
fi

# Sort longest tests first, then assign each to the least-loaded shard.
tab=$(printf '\t')
awk -F '\t' -v cache_file="$cache_file" '
	FILENAME == cache_file {
		duration[$1] = $2
		next
	}
	{ print (($0 in duration) ? duration[$0] : 1) "\t" $0 }
' "$cache_file" "$output_dir/tests" |
	sort -t "$tab" -k1,1nr -k2,2 >"$output_dir/weighted-tests"

awk -F '\t' -v jobs="$shard_count" '
	{
		shard = 1
		for (i = 2; i <= jobs; i++)
			if (load[i] < load[shard]) shard = i

		printf "%d\t%s\n", shard - 1, $2
		load[shard] += $1
	}
' "$output_dir/weighted-tests" >"$output_dir/schedule"

status=0
pids=""
shard=0
# Run one Google Test process per shard and stream its output as it runs.
while [ "$shard" -lt "$shard_count" ]; do
	filter=$(awk -F '\t' -v shard="$shard" '
		$1 == shard {
			if (count++) printf ":"
			printf "%s", $2
		}
	' "$output_dir/schedule")
	(
		./sample_test --gtest_filter="$filter" --gtest_color=yes 2>&1
		echo "$?" >"$output_dir/status-$shard"
	) | tee "$output_dir/shard-$shard.log" &
	pids="$pids $!"
	shard=$((shard + 1))
done

shard=0
# Preserve failures from each test process; tee's status alone is not enough.
for pid in $pids; do
	wait "$pid" || status=1
	test_status=$(cat "$output_dir/status-$shard" 2>/dev/null || echo 1)
	[ "$test_status" -eq 0 ] || status=1
	shard=$((shard + 1))
done

# Refresh cached runtimes from Google Test's per-test result lines.
if awk -F '\t' -v cache_file="$cache_file" '
	FILENAME == cache_file { duration[$1] = $2; next }
	{
		gsub(/\033\[[0-9;]*m/, "")
		line = $0
		if (line ~ /^\[       OK \] /)
			sub(/^\[       OK \] /, "", line)
		else if (line ~ /^\[  FAILED  \] /)
			sub(/^\[  FAILED  \] /, "", line)
		else
			next

		if (line ~ / \([0-9]+ ms\)$/) {
			name = line
			sub(/ \([0-9]+ ms\)$/, "", name)
			sub(/^.* \(/, "", line)
			sub(/ ms\)$/, "", line)
			duration[name] = line
		}
	}
	END {
		for (name in duration) print name "\t" duration[name]
	}
' "$cache_file" "$output_dir"/shard-*.log >"$cache_tmp" &&
	mv "$cache_tmp" "$cache_file"; then
	:
else
	echo "Could not update test timing cache" >&2
	status=1
fi

# Combine per-shard results into one final summary.
awk '
	{ gsub(/\033\[[0-9;]*m/, "") }
	/^\[       OK \] .* \([0-9]+ ms\)$/ { passed++ }
	/^\[  FAILED  \] .* \([0-9]+ ms\)$/ {
		failed++
		print "  " $0
	}
	/^\[  SKIPPED \] .* \([0-9]+ ms\)$/ { skipped++ }
	END {
		printf "[==========] %d tests ran across %d shards.\n", passed + failed + skipped, ARGC - 1
		printf "\033[32m[  PASSED  ]\033[0m %d tests.\n", passed
		printf "\033[31m[  FAILED  ]\033[0m %d tests.\n", failed
		printf "\033[33m[  SKIPPED ]\033[0m %d tests.\n", skipped
	}
' "$output_dir"/shard-*.log

exit "$status"
