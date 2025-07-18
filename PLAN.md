# Implementation Plan: Git-Tag-Based Semversioning with CI/CD

## Project Overview
Transform the Image Batch Processor (IBP) codebase to implement a complete git-tag-based semantic versioning system with automated CI/CD, comprehensive testing, and multiplatform release automation.

## Current State Analysis
- **Language**: C++ Qt application with Python packaging using Hatch
- **Build System**: CMake for C++ + Hatch for Python packaging
- **Current Version**: 2.1.0 (from VERSION.TXT)
- **Existing CI/CD**: Basic GitHub Actions workflow with some gaps
- **Testing**: Basic Python packaging tests, no comprehensive test suite

## Technical Architecture Decisions

### 1. Semantic Versioning Strategy
- **Version Source**: Git tags as single source of truth
- **Version Format**: SemVer 2.0.0 (X.Y.Z with optional pre-release and build metadata)
- **Automation**: GitVersion or similar tool for automatic version calculation
- **Version Propagation**: 
  - Python package version via `hatch-vcs`
  - C++ version via CMake configuration
  - Update VERSION.TXT automatically

### 2. Branching and Tagging Strategy
- **Main Branch**: `master` (production-ready code)
- **Development**: Feature branches → master
- **Release Process**: 
  - Tag format: `v{major}.{minor}.{patch}[-{prerelease}]` (e.g., `v2.1.0`, `v2.2.0-beta.1`)
  - Pre-release tags: `v{version}-alpha.{N}`, `v{version}-beta.{N}`, `v{version}-rc.{N}`
  - Development builds: Automatic versioning with commit hash

### 3. Testing Architecture
- **Unit Tests**: C++ component tests using Google Test
- **Integration Tests**: Python packaging and CLI tests
- **End-to-End Tests**: Full application functionality tests
- **Performance Tests**: Image processing benchmarks
- **Cross-Platform Tests**: Linux, macOS, Windows validation

## Implementation Phases

### Phase 1: Version Management Infrastructure
**Tasks:**
1. **Update pyproject.toml for dynamic versioning**
   - Configure `hatch-vcs` for git-tag-based versioning
   - Remove hardcoded version references

2. **Enhance CMake version handling**
   - Update `GetVersion.cmake` to support git-tag-based versioning
   - Add fallback to VERSION.TXT for non-git environments
   - Generate version headers with git metadata

3. **Create version synchronization system**
   - Script to update VERSION.TXT from git tags
   - Validate version consistency across all systems

### Phase 2: Comprehensive Test Suite
**Tasks:**
1. **C++ Unit Test Framework**
   - Integrate Google Test into CMake build
   - Create test targets for core libraries (`imgproc`, `misc`, `widgets`)
   - Add plugin testing framework

2. **Python Test Enhancement**
   - Expand packaging tests
   - Add CLI integration tests
   - Create test data and fixtures

3. **Cross-Platform Test Suite**
   - Docker-based testing environments
   - Platform-specific test cases
   - Performance and memory leak tests

### Phase 3: Local Build and Release Scripts
**Tasks:**
1. **Build Automation Scripts**
   - `build.sh`/`build.ps1`: Cross-platform build script
   - `test.sh`/`test.ps1`: Comprehensive test runner
   - `release.sh`/`release.ps1`: Local release preparation

2. **Release Management Tools**
   - Version bump utilities
   - Changelog generation
   - Release notes automation

3. **Developer Tools**
   - Development environment setup
   - Code formatting and linting
   - Pre-commit hooks

### Phase 4: GitHub Actions CI/CD Pipeline
**Tasks:**
1. **Multi-Platform Build Matrix**
   - Linux (Ubuntu 20.04, 22.04)
   - macOS (Intel, Apple Silicon)
   - Windows (MSVC, MinGW)

2. **Automated Testing Pipeline**
   - Unit tests on all platforms
   - Integration tests
   - Performance benchmarks
   - Security scanning

3. **Release Automation**
   - Automatic releases on git tags
   - Binary artifact generation
   - Package publishing to PyPI
   - GitHub Releases with changelogs

### Phase 5: Multiplatform Binary Distribution
**Tasks:**
1. **Binary Packaging**
   - Self-contained executables
   - Platform-specific installers
   - Portable/AppImage formats

2. **Distribution Channels**
   - GitHub Releases
   - PyPI package distribution
   - Platform-specific repositories (brew, apt, chocolatey)

3. **Installation Automation**
   - Easy installation scripts
   - Package manager integration
   - User documentation

## Technical Specifications

### Version Management
- **Git Tags**: Primary version source
- **hatch-vcs**: Python package versioning
- **CMake**: C++ version propagation
- **VERSION.TXT**: Backup for non-git builds

### Testing Strategy
- **Test Coverage**: Minimum 80% for core components
- **Test Types**:
  - Unit tests (C++ with Google Test)
  - Integration tests (Python with pytest)
  - End-to-end tests (CLI and GUI)
  - Performance tests (benchmarking)
  - Security tests (static analysis)

### Build Process
- **CMake**: C++ compilation and linking
- **Hatch**: Python packaging and distribution
- **Cross-compilation**: Support for multiple architectures
- **Dependency Management**: Automatic dependency resolution

### Release Process
- **Trigger**: Git tag push
- **Validation**: All tests must pass
- **Artifacts**: 
  - Python wheels for all platforms
  - Standalone executables
  - Source distributions
  - Documentation

## Risk Assessment and Mitigation

### Technical Risks
1. **Cross-platform compatibility**: Mitigated by comprehensive CI testing
2. **Version synchronization**: Addressed by automated scripts and validation
3. **Build dependency management**: Resolved through containerization and caching

### Process Risks
1. **Release automation failures**: Mitigated by rollback procedures and manual override
2. **Version conflicts**: Prevented by strict validation and testing
3. **Security vulnerabilities**: Addressed through automated scanning and updates

## Success Criteria
- ✅ Git tags drive all version numbers consistently
- ✅ Comprehensive test suite with >80% coverage
- �� Automated multiplatform builds and releases
- ✅ Easy local development and testing workflow
- ✅ Binary distributions for all major platforms
- ✅ Seamless CI/CD pipeline with git tag releases
- ✅ User-friendly installation process

## Future Considerations
- **Automatic dependency updates**: Dependabot integration
- **Security scanning**: Regular vulnerability assessments
- **Performance monitoring**: Automated performance regression detection
- **Documentation**: Auto-generated API documentation
- **Containerization**: Docker images for consistent deployment