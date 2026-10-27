#!/bin/sh

set -u

if [ -n "${TEST_JOBS:-}" ]; then
	jobs=$TEST_JOBS
else
	jobs=$(nproc) || {
		echo "Could not detect available CPUs; set TEST_JOBS explicitly" >&2
		exit 2
	}
fi
case "$jobs" in
	''|*[!0-9]*) echo "Shard count must be a positive integer" >&2; exit 2;;
esac
if [ "$jobs" -lt 1 ]; then
	echo "Shard count must be a positive integer" >&2
	exit 2
fi

test_list=$(./sample_test --gtest_list_tests --gtest_filter='*') || {
	echo "Could not list sample_test cases" >&2
	exit 2
}
cache_file=.sample_test_timings.tsv
test_names=$(mktemp) || {
	echo "Could not create temporary test list" >&2
	exit 2
}
printf '%s\n' "$test_list" | awk '
	/^[^[:space:]].*\.$/ {
		suite = $1
		sub(/\.$/, "", suite)
		next
	}
	/^  [^#[:space:]]/ { print suite "." $1 }
' >"$test_names"
test_count=$(awk 'END { print NR + 0 }' "$test_names")
if [ "$test_count" -lt 1 ]; then
	rm -f "$test_names"
	echo "sample_test did not report any tests" >&2
	exit 2
fi

shard_count=$jobs
if [ "$shard_count" -gt "$test_count" ]; then
	shard_count=$test_count
fi

output_dir=$(mktemp -d) || {
	rm -f "$test_names"
	echo "Could not create temporary directory for test output" >&2
	exit 2
}
cache_tmp=./.sample_test_timings.tsv.tmp.$$
trap 'rm -rf "$output_dir"; rm -f "$test_names" "$cache_tmp"' EXIT

if [ ! -f "$cache_file" ]; then
	: >"$cache_file"
fi
awk -F '\t' -v shard_count="$shard_count" '
	FILENAME == ARGV[1] {
		if (NF >= 2 && $2 ~ /^[0-9]+$/) duration[$1] = $2
		next
	}
	NF {
		count++
		name[count] = $0
		weight[count] = (($0 in duration) ? duration[$0] : 1)
	}
	END {
		for (step = 1; step <= count; step++) {
			longest = 0
			for (i = 1; i <= count; i++) {
				if (!assigned[i] && (longest == 0 || weight[i] > weight[longest]))
					longest = i
			}

			least_loaded = 1
			for (i = 2; i <= shard_count; i++) {
				if (load[i] < load[least_loaded])
					least_loaded = i
			}

			load[least_loaded] += weight[longest]
			bucket[longest] = least_loaded
			assigned[longest] = 1
		}

		for (pass = 1; pass <= count * 2; pass++) {
			current_max = 0
			for (i = 1; i <= shard_count; i++)
				if (load[i] > current_max) current_max = load[i]
			best_max = current_max
			best_type = 0

			for (i = 1; i <= count; i++) {
				source = bucket[i]
				for (destination = 1; destination <= shard_count; destination++) {
					if (destination == source) continue
					candidate_max = 0
					for (b = 1; b <= shard_count; b++) {
						candidate = load[b]
						if (b == source) candidate -= weight[i]
						if (b == destination) candidate += weight[i]
						if (candidate > candidate_max) candidate_max = candidate
					}
					if (candidate_max < best_max) {
						best_max = candidate_max
						best_type = 1
						best_i = i
						best_bucket = destination
					}
				}
			}

			for (i = 1; i <= count; i++) {
				for (j = i + 1; j <= count; j++) {
					if (bucket[i] == bucket[j]) continue
					candidate_max = 0
					for (b = 1; b <= shard_count; b++) {
						candidate = load[b]
						if (b == bucket[i])
							candidate += weight[j] - weight[i]
						if (b == bucket[j])
							candidate += weight[i] - weight[j]
						if (candidate > candidate_max) candidate_max = candidate
					}
					if (candidate_max < best_max) {
						best_max = candidate_max
						best_type = 2
						best_i = i
						best_j = j
					}
				}
			}

			if (best_type == 0) break
			if (best_type == 1) {
				source = bucket[best_i]
				load[source] -= weight[best_i]
				bucket[best_i] = best_bucket
				load[best_bucket] += weight[best_i]
			} else {
				source = bucket[best_i]
				destination = bucket[best_j]
				load[source] += weight[best_j] - weight[best_i]
				load[destination] += weight[best_i] - weight[best_j]
				bucket[best_i] = destination
				bucket[best_j] = source
			}
		}

		for (i = 1; i <= count; i++)
			printf "%d\t%s\n", bucket[i] - 1, name[i]
	}
' "$cache_file" "$test_names" >"$output_dir/schedule.tsv"
rm -f "$test_names"

status=0
shard=0
pids=""
while [ "$shard" -lt "$shard_count" ]; do
	filter=$(awk -F '\t' -v shard="$shard" '
		$1 == shard {
			if (count++) printf ":"
			printf "%s", $2
		}
	' "$output_dir/schedule.tsv")
	(
		./sample_test --gtest_filter="$filter" --gtest_color=yes 2>&1
		test_status=$?
		printf '%s\n' "$test_status" >"$output_dir/shard-$shard.status"
	) | tee "$output_dir/shard-$shard.log" &
	pids="$pids $!"
	shard=$((shard + 1))
done

shard=0
for pid in $pids; do
	if ! wait "$pid"; then
		status=1
	fi
	test_status=$(cat "$output_dir/shard-$shard.status" 2>/dev/null || echo 1)
	if [ "$test_status" -ne 0 ]; then
		status=1
	fi
	shard=$((shard + 1))
done

if ! awk -F '\t' -v cache_file="$cache_file" '
	FILENAME == cache_file {
		if (NF >= 2 && $2 ~ /^[0-9]+$/) duration[$1] = $2
		next
	}
	{
		gsub(/\033\[[0-9;]*m/, "")
		line = $0
		if (line ~ /^\[       OK \] /)
			sub(/^\[       OK \] /, "", line)
		else if (line ~ /^\[  FAILED  \] /)
			sub(/^\[  FAILED  \] /, "", line)
		else
			next

		if (line !~ / \([0-9]+ ms\)$/) next
		name = line
		sub(/ \([0-9]+ ms\)$/, "", name)
		milliseconds = line
		sub(/^.* \(/, "", milliseconds)
		sub(/ ms\)$/, "", milliseconds)
		duration[name] = milliseconds
	}
	END {
		for (name in duration) print name "\t" duration[name]
	}
' "$cache_file" "$output_dir"/shard-*.log >"$cache_tmp"; then
	echo "Could not update test timing cache" >&2
	status=1
elif ! mv "$cache_tmp" "$cache_file"; then
	echo "Could not save test timing cache" >&2
	status=1
fi

awk '
	{ gsub(/\033\[[0-9;]*m/, "") }
	/^\[       OK \] .* \([0-9]+ ms\)$/ { passed++ }
	/^\[  FAILED  \] .* \([0-9]+ ms\)$/ {
		failed++
		print "  " $0
	}
	/^\[  SKIPPED \] .* \([0-9]+ ms\)$/ { skipped++ }
	END {
		total = passed + failed + skipped
		printf "[==========] %d tests ran across %d shards.\n", total, ARGC - 1
		printf "\033[32m[  PASSED  ]\033[0m %d tests.\n", passed
		printf "\033[31m[  FAILED  ]\033[0m %d tests.\n", failed
		printf "\033[33m[  SKIPPED ]\033[0m %d tests.\n", skipped
	}
' "$output_dir"/shard-*.log

exit "$status"
