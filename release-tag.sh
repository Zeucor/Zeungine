#!/bin/bash

if [ -z "$1" ]; then
	echo "Only Repo Maintainers should run this"
	echo " # Commit InstallPrefix.cmake increment first as vx.x.x.x and then call this script"
	echo "Usage: $0 <tag>"
	exit 1
fi

TAG="$1"

git fetch --tags --force

PREV_HASH=$(git for-each-ref --sort=-creatordate --format '%(objectname)' refs/tags | sed -n 2p)
PREV_TAG=$(git for-each-ref --sort=-creatordate --format '%(refname:strip=2)' refs/tags | sed -n 2p)
CURR_HASH=$(git rev-parse HEAD)

echo "## Changes since $PREV_TAG" > CURVER_CHANGELOG.md
echo "" >> CURVER_CHANGELOG.md
git log $PREV_HASH..$CURR_HASH --pretty=format:"- %h %s (%an, %ar)" >> CURVER_CHANGELOG.md
echo "" >> CURVER_CHANGELOG.md

(cat CURVER_CHANGELOG.md && cat CHANGELOG.md) > NEW_CHANGELOG.md
mv NEW_CHANGELOG.md CHANGELOG.md

git add CHANGELOG.md CURVER_CHANGELOG.md
git commit -m "Update CHANGELOG for $TAG"

git add cmake/InstallPrefix.cmake
git commit -m "$TAG"

git tag "$TAG"
git push origin main --follow-tags

