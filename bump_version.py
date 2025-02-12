import re
from pathlib import Path

from git import GitCommandError, Repo
from semver import Version

MAIN_BRANCH = "main"
DEV_BRANCH = "dev"

VERSION_OCCURANCES = [
    ("README.md", r"(source: github://bernikr/esphome-wienernetze-im150-smartmeter@v)(\S+)()", 1),
    ("components/im150/im150.h", r'(static const char \*IM150_VERSION = ")(\S+)(";)', 1),
    ("pyproject.toml", r'(version = ")(\S+)(")', 1),
]

versions = set()
has_wanings = False

for filename, regex, occurrences in VERSION_OCCURANCES:
    with Path(__file__).parent.joinpath(filename).open("r") as f:
        res = re.findall(regex, f.read())
    if len(res) != occurrences:
        print(f"WARNING: version occurance mismatch in {filename}")
        has_wanings = True
    versions.update(r[1] for r in res)

if len(versions) != 1:
    print(f"WARNING: multiple versions found: {versions}")
    has_wanings = True

version = max(Version.parse(v) for v in versions)
print(f"Current version: {version}")
print("Possible next versions:")
print(f"  1 or M:   {version.next_version('major')}")
print(f"  2 or m:   {version.next_version('minor')}")
print(f"  3 or p:   {version.next_version('patch')}")
print(f"  4 or b:   {version.next_version('prerelease', 'beta')}")

next_version = version.next_version("prerelease", "beta") if version.prerelease else version.next_version("patch")
print(f"Default version: {next_version} [Enter to confirm] or enter a new version")
user_input = input()
if user_input:
    if user_input.lower() in {"1", "M"}:
        next_version = version.next_version("major")
    elif user_input.lower() in {"2", "m"}:
        next_version = version.next_version("minor")
    elif user_input.lower() in {"3", "p"}:
        next_version = version.next_version("patch")
    elif user_input.lower() in {"4", "b"}:
        next_version = version.next_version("prerelease", "beta")
    else:
        next_version = Version.parse(user_input)

print(f"Entered version: {next_version}")
if not next_version > version:
    print("WARNING: new version should be greater than current version")
    has_wanings = True

repo = Repo(Path(__file__).parent)
if repo.is_dirty():
    print("WARNING: repo is dirty, please commit or stage changes before continuing")
    input("Press enter to continue")

for filename, regex, _ in VERSION_OCCURANCES:
    with Path(__file__).parent.joinpath(filename).open("r+") as f:
        res = re.sub(regex, f"\\g<1>{next_version}\\g<3>", f.read())
        f.seek(0)
        f.write(res)
        f.truncate()

if has_wanings:
    print("WARNING: there were warnings, please check the output before contiuning")
    input("Press enter to continue")

res = input("Do you want to commit the changes? [y/N] ")
if res.lower() in {"y", "yes"}:
    repo.git.add(*{filename for filename, _, _ in VERSION_OCCURANCES})
    repo.git.commit("-m", f"Bump version to {next_version}")
    repo.create_tag(f"v{next_version}", message=f"Bump version to {next_version}")
    print("changes committed and created tag")
    if not next_version.prerelease:
        repo.git.checkout(MAIN_BRANCH)
        try:
            repo.git.merge(DEV_BRANCH, ff_only=True)
        except GitCommandError:
            print(f"WARNING: merge into {MAIN_BRANCH} failed, please merge manually")
        finally:
            repo.git.checkout(DEV_BRANCH)
    res = input("Do you want to push the changes? [y/N] ")
    if res.lower() in {"y", "yes"}:
        repo.git.push(follow_tags=True)
        if not next_version.prerelease:
            repo.git.push("origin", MAIN_BRANCH)
        print("changes pushed")
