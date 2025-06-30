#!/usr/bin/env python3

from itertools import batched
import re
import zeekpkg

with open("/Users/tim/Desktop/projects/packages/aggregate.meta", "r") as f:
    all_packages = f.read()

packages = re.split(r'^\[(.*?\/.*?)\]', all_packages, flags=re.MULTILINE)
packages_to_test = []

for n, p in batched(packages[1:], n=2):
    lines = p.split('\n')
    if len(lines) < 2:
        continue;

    build_command = ''
    url = ''

    for l in lines:
        m = re.match(r"^build_command = (.*)", l)
        if m:
            if m.group(1).find('bro_') == -1:
                build_command = m.group(1)

        m = re.match(r"^url = (.*)", l)
        if m:
            url = m.group(1)

    if build_command and url:
        packages_to_test.append([n, url, build_command])

mgr = zeekpkg.manager.Manager(state_dir='/Users/tim/Desktop/zeek-install/var/lib/zkg',
                              script_dir='/Users/tim/Desktop/zeek-install/share/zeek/site',
                              plugin_dir='/Users/tim/Desktop/zeek-install/lib/zeek/plugins',
                              zeek_dist='/Users/tim/Desktop/projects/zeek')

for p in packages_to_test:
    if p[0].find('hart-ip') != -1:
        continue

    print(f'Installing {p[0]} from {p[1]}')
    print(mgr.install(p[1]))
    print('\n\n')
