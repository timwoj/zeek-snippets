#!/usr/bin/env python3

import os
import pygit2
import sys
import re
import subprocess
import argparse

zeek_repo = re.compile('github\.com[:/]zeek')
zeek_cmake = re.compile('zeek/cmake')
seen = []

# Some globals that'll be filled in when we start up
cmake_branch = 'topic/johanna/openssl-cmake-update'
cmake_HEAD = ''
dev_branch = ''

# Needed to get the authentication information for git. Otherwise pygit2 can't talk to
# the repositories.
class GitRemoteCallbacks(pygit2.RemoteCallbacks):

    def credentials(self, url, username_from_url, allowed_types):
        if allowed_types & pygit2.credentials.GIT_CREDENTIAL_USERNAME:
            return pygit2.Username("git")
        elif allowed_types & pygit2.credentials.GIT_CREDENTIAL_SSH_KEY:
            return pygit2.KeypairFromAgent("git")
        else:
            return None

# Returns whether the inner submodule had changes. This indicates the outer submodule
# needs to make a commit to include them.
def create_dev_branches(path):

    try:
        repo_path = pygit2.discover_repository(path)
        repo = pygit2.Repository(repo_path)
        origin = repo.remotes['origin']

        found_cmake_updates = []
        if origin.url in seen:
            return False

        seen.append(origin.url)

        # Skip this submodule altogether if this is not a zeek repository
        if not zeek_repo.search(origin.url):
            return False

        origin.fetch(callbacks=GitRemoteCallbacks())

        # If this submodule is a cmake submodule, do some things
        if zeek_cmake.search(origin.url):
            # If the submodule is already pointing at the master HEAD, skip it and return false
            if repo.head.target == cmake_HEAD:
                return False

            # If not, reset to master HEAD and return true
            commit = repo.get(cmake_HEAD)
            repo.reset(commit.oid, pygit2.GIT_RESET_HARD)
            return True

        # if this submodule isn't a cmake submodule, get the list of submodules from this
        # one, and traverse down into them.
        else:
            for child in repo.listall_submodules():
                if create_dev_branches(os.path.join(path, child)):
                    found_cmake_updates.append(child)

        # If we found a cmake update down the tree somewhere, we need to check out a branch
        # and commit the submodule changes that caused it
        if found_cmake_updates:
            # Try to find the branch we're using to do this update
            branch = repo.lookup_branch(dev_branch)
            branch_obj = None
            branch_ref = None

            # If the branch doesn't exist, create it and push it
            if not branch:
                # This feels convoluted as heck but I can't find a better way to do it via
                # pygit2 methods. You need the master branch to get master's latest commit,
                # since you need to that commit as the basis for the new branch.
                # Once you've made the branch, you have to look it up again to get the
                # reference, so you can push it to the remote.
                master_branch = repo.lookup_branch('master')
                master_commit = repo.get(master_branch.target)
                branch = repo.branches.create(dev_branch, master_commit)
                [branch_commit, branch_ref] = repo.resolve_refish(dev_branch)
                repo.remotes['origin'].push(branch_ref.name, callbacks=GitRemoteCallbacks())
            else:
                [branch_commit, branch_ref] = repo.resolve_refish(dev_branch)

            # Go ahead and check out that branch
            repo.checkout(branch)

            # This part is a little convoluted too. Grab the index current index from the
            # repo, add the updated paths to it, and write it to the repo. This is the
            # equivalent of doing 'git add' on all of the paths.
            index = repo.index
            for child_path in found_cmake_updates:
                index.add(child_path)
            index.write()

            # Once you have the index written, get the tree from the index and create
            # a commit tied to the head commit of the repo (which is on the branch),
            # and then push the branch back to the remote, which will also push the
            # commit.
            tree = index.write_tree()
            commit = repo.create_commit(branch.name, repo.default_signature, repo.default_signature,
                                        'Updating cmake submodule [nomail]', tree, [repo.head.target.hex])
            repo.remotes['origin'].push(branch_ref.name, callbacks=GitRemoteCallbacks())

            return True
    except:
        print(f'failure at {path}: {sys.exc_info()}')

    return False

def merge_dev_branches(path):

    try:
        repo_path = pygit2.discover_repository(path)
        repo = pygit2.Repository(repo_path)
        origin = repo.remotes['origin']

        found_cmake_updates = []
        if origin.url in seen:
            return False

        seen.append(origin.url)

        # Skip this submodule altogether if this is not a zeek repository
        if not zeek_repo.search(origin.url):
            return False

        origin.fetch(callbacks=GitRemoteCallbacks())

        # We don't care about the cmake repos because those have been pushed to the commit
        # we wanted already.
        if not zeek_cmake.search(origin.url):
            for child in repo.listall_submodules():
                if merge_dev_branches(os.path.join(path, child)):
                    found_dev_branches.append(child)

    except:
        print(f'failure at {path}: {sys.exc_info()}')

    return False


# Start at the top of course, and throw an error if we're not in the right directory.
top_path = pygit2.discover_repository(os.getcwd())
if not top_path:
    print('ERROR: Must be in a top-level zeek repository to run this script')
    sys.exit(1)

top = pygit2.Repository(top_path)
origin = top.remotes['origin']
if not origin.url.endswith(':zeek/zeek') and not origin.url.endswith(':zeek/zeek.git'):
    print('ERROR: Must be in a top-level zeek repository to run this script')
    sys.exit(1)

parser = argparse.ArgumentParser(description='Merges a change to the cmake repo across all submodules')
parser.add_argument('--dev_branch', help='The name of a branch used in each of the submodules. For staging mode, this is the branch the commits will be staged to. For merge mode, this is the branch that will be merged into master.', required=True, action='store')
parser.add_argument('--cmake_branch', help='The name of the branch in the cmake repo to merge. This will be used to pull the commit to reset to, and should have already been rebased on the master branch for that repo. This argument is ignored in merge mode. This can be set to \'master\' if the commit has already been merged', action='store')
parser.add_argument('--mode', choices=['stage','merge'], help='The mode the tool will run in. In \'stage\' mode, the tool will update each cmake submodule in the tree to the HEAD commit on the branch specified by the --cmake_branch option, and then create new branches named by the --dev-branch option in each repository upwards from those changes. In \'merge\' mode, the tool will merge all of the branches named by the --dev-branch option into the submodules\' master branches.', required=True)

args = parser.parse_args()

dev_branch = args.dev_branch
remote_result = subprocess.run(['git','ls-remote','https://github.com/zeek/cmake',cmake_branch], capture_output=True)
cmake_HEAD = remote_result.stdout.decode('ascii').split()[0]

if parser.mode == 'stage':
    create_dev_branches(os.getcwd())
elif parser.mode == 'merge':
    merge_dev_branches(os.getcwd())
