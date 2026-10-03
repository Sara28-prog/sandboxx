#!/bin/bash

echo "=== SandBoxX Cleanup Script ==="
echo "Cleaning up dangling cgroups and ephemeral mounts..."

# Unmount any stale mounts
if mountpoint -q ./rootfs/proc 2>/dev/null; then
    umount -l ./rootfs/proc
fi
if mountpoint -q ./rootfs/sys 2>/dev/null; then
    umount -l ./rootfs/sys
fi
if mountpoint -q ./rootfs/tmp 2>/dev/null; then
    umount -l ./rootfs/tmp
fi

# Remove cgroups
if [ -d /sys/fs/cgroup/sandboxx ]; then
    rmdir /sys/fs/cgroup/sandboxx/* 2>/dev/null || true
fi

echo "Cleanup completed."
