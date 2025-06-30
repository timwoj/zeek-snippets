#!/usr/bin/env python3

from __future__ import annotations

import argparse
import git
import os
import re
from collections.abc import Sequence

def main(argv: Sequence[str] | None = None) -> int:
    # This should be the repo we're running pre-commit on.
    repo = git.Repo('.')

    # rev_list returns a new-line separated list of commits.
    # split it and grab the last one, which is the first commit
    # since the branch started.
    first_commit = repo.git.rev_list('HEAD', '^master').split()[-1]

    # diff_trace returns a new-line separate list of the files
    # changed since the commit found above. We only care about
    # files that are in a btest baseline.
    file_list = sorted(repo.git.diff_tree('--no-commit-id', '--name-only', first_commit, '-r').split())

    full_file_list = sorted(repo.git.ls_tree('-r', '--name-only', 'HEAD').split())
    zam_baselines = [x for x in full_file_list if x.startswith('testing/btest/Baseline.zam')]

    retval = 0

    for f in file_list:
        # Only look for files in the main baseline
        if not f.startswith('testing/btest/Baseline/'):
            continue

        zam_f = f.replace('/Baseline/', '/Baseline.zam/', 1)

        # Skip if this file is already in the list of changed files
        if zam_f in file_list:
            continue

        test_name = f.split('/')[3]

        # If the zam file already exists, it might need to be updated.
        if zam_f in zam_baselines:
            print(f'ZAM baseline may need update for {test_name}')

        # If the test name ends with a -#, this is a subtest. Skip subtests
        # for the rest of this. Check to see if the outer test exists though
        # just in case someone was weird and named a test with a number at
        # the end.
        fr = re.match(r'(.*)-\d+', f)
        if fr and fr.groups()[0] in full_file_list:
            continue

        # Check if the test has ZAM force-enabled, because that means we might
        # need to add new baselines.
        test_path = f'testing/btest/{test_name.replace(".","/")}'
        test_file = [x for x in full_file_list if x.startswith(test_path)][0]
        if 'test "${ZEEK_ZAM}" = "1"' in open(test_file).read():
            print(f'New ZAM baseline needs to be generated for {test_name}')
            retval = 1

    return retval


if __name__ == '__main__':
    raise SystemExit(main())
