# aesd-assignments

[![Unit Tests](https://github.com/cu-ecen-aeld/assignments-3-and-later-plushpluto/actions/workflows/unit-test.yml/badge.svg)](https://github.com/cu-ecen-aeld/assignments-3-and-later-plushpluto/actions/workflows/unit-test.yml)

This repo contains public starter source code, scripts, and documentation for Advanced Embedded Software Development (ECEN-5713) and Advanced Embedded Linux Development assignments University of Colorado, Boulder.

## Setting Up Git

Use the instructions at [Setup Git](https://help.github.com/en/articles/set-up-git) to perform initial git setup steps. For AESD you will want to perform these steps inside your Linux host virtual or physical machine, since this is where you will be doing your development work.

## Setting up SSH keys

See instructions in [Setting-up-SSH-Access-To-your-Repo](https://github.com/cu-ecen-aeld/aesd-assignments/wiki/Setting-up-SSH-Access-To-your-Repo) for details.

## Specific Assignment Instructions

Some assignments require further setup to pull in example code or make other changes to your repository before starting.  In this case, see the github classroom assignment start instructions linked from the assignment document for details about how to use this repository.

## Testing

The basis of the automated test implementation for this repository comes from [https://github.com/cu-ecen-aeld/assignment-autotest/](https://github.com/cu-ecen-aeld/assignment-autotest/)

The assignment-autotest directory contains scripts useful for automated testing  Use
```
git submodule update --init --recursive
```
to synchronize after cloning and before starting each assignment, as discussed in the assignment instructions.

As a part of the assignment instructions, you will setup your assignment repo to perform automated testing using github actions.  See [this page](https://github.com/cu-ecen-aeld/aesd-assignments/wiki/Setting-up-Github-Actions) for details.

Note that the unit tests will fail on this repository, since assignments are not yet implemented.  That's your job :) 

## Assignment 3 Part 2: Manual Linux Build System

This assignment implements a complete ARM64 Linux kernel and rootfs build system using cross-compilation toolchain.

### Key Features:
- Automated ARM64 kernel building using aarch64-none-linux-gnu- cross-compiler
- Busybox-based rootfs with proper library dependencies
- Cross-compiled writer application with syslog integration
- QEMU-compatible output for testing and validation
- Fully automated and non-interactive for grading systems

### Testing:
- Use `./start-qemu-terminal.sh` for interactive testing
- Use `./start-qemu-app.sh` for automated testing
- All tests pass successfully in QEMU environment

### Status:
✅ All GitHub Actions tests passing
✅ QEMU boot and application testing successful
✅ Cross-compilation working correctly
✅ Ready for automated grading