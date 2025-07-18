#!/bin/bash
# this_file: release.sh

# Release preparation script for Image Batch Processor
# Usage: ./release.sh [version]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Print usage
usage() {
    cat << EOF
Usage: $0 [VERSION]

Release preparation script for Image Batch Processor

ARGUMENTS:
    VERSION     Version to release (e.g., 2.2.0, 2.2.0-beta.1)
                If not provided, will auto-increment based on git tags

OPTIONS:
    -h, --help      Show this help message
    -n, --dry-run   Show what would be done without actually doing it
    -f, --force     Force release even if working directory is dirty

EXAMPLES:
    $0 2.2.0        # Release version 2.2.0
    $0 2.2.0-beta.1 # Release beta version
    $0              # Auto-increment version
EOF
}

# Default options
DRY_RUN=false
FORCE=false
VERSION=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -n|--dry-run)
            DRY_RUN=true
            shift
            ;;
        -f|--force)
            FORCE=true
            shift
            ;;
        -*)
            log_error "Unknown option: $1"
            usage
            exit 1
            ;;
        *)
            if [ -z "$VERSION" ]; then
                VERSION="$1"
            else
                log_error "Too many arguments"
                usage
                exit 1
            fi
            shift
            ;;
    esac
done

# Validate semantic version format
validate_version() {
    local version="$1"
    if [[ ! "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9.-]+)?$ ]]; then
        log_error "Invalid version format: $version"
        log_error "Expected format: X.Y.Z or X.Y.Z-prerelease"
        exit 1
    fi
}

# Auto-increment version based on git tags
auto_increment_version() {
    local latest_tag=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")
    
    # Remove 'v' prefix if present
    latest_tag=${latest_tag#v}
    
    # Split version into components
    IFS='.' read -r major minor patch <<< "$latest_tag"
    
    # Remove any pre-release suffix from patch
    patch=${patch%%-*}
    
    # Increment patch version
    patch=$((patch + 1))
    
    echo "$major.$minor.$patch"
}

# Get current version
get_current_version() {
    local current_tag=$(git describe --tags --exact-match HEAD 2>/dev/null || echo "")
    
    if [ -n "$current_tag" ]; then
        echo "${current_tag#v}"
    else
        echo ""
    fi
}

# Check if working directory is clean
check_git_status() {
    if [ "$FORCE" = true ]; then
        return 0
    fi
    
    if ! git diff --quiet HEAD; then
        log_error "Working directory is not clean"
        log_error "Please commit your changes or use --force"
        exit 1
    fi
}

# Update VERSION.TXT file
update_version_file() {
    local version="$1"
    
    if [ "$DRY_RUN" = true ]; then
        log_info "Would update VERSION.TXT to $version"
        return 0
    fi
    
    echo "$version" > VERSION.TXT
    log_success "Updated VERSION.TXT to $version"
}

# Generate changelog entry
generate_changelog() {
    local version="$1"
    local previous_tag=$(git describe --tags --abbrev=0 HEAD^ 2>/dev/null || echo "")
    
    if [ -z "$previous_tag" ]; then
        log_warning "No previous tag found, skipping changelog generation"
        return 0
    fi
    
    local changelog_entry=""
    changelog_entry+="## [$version] - $(date -u +%Y-%m-%d)\n\n"
    
    # Get commit messages since last tag
    local commits=$(git log --oneline "$previous_tag"..HEAD --pretty=format:"- %s" 2>/dev/null || echo "")
    
    if [ -n "$commits" ]; then
        changelog_entry+="### Changes\n"
        changelog_entry+="$commits\n\n"
    fi
    
    # Add comparison link
    changelog_entry+="[Full Changelog](https://github.com/twardoch/imagebatchprocessor/compare/${previous_tag}...v${version})\n\n"
    
    if [ "$DRY_RUN" = true ]; then
        log_info "Would add changelog entry:"
        echo -e "$changelog_entry"
        return 0
    fi
    
    # Create CHANGELOG.md if it doesn't exist
    if [ ! -f CHANGELOG.md ]; then
        cat > CHANGELOG.md << EOF
# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

EOF
    fi
    
    # Insert new entry after the header
    local temp_file=$(mktemp)
    head -n 6 CHANGELOG.md > "$temp_file"
    echo -e "$changelog_entry" >> "$temp_file"
    tail -n +7 CHANGELOG.md >> "$temp_file"
    mv "$temp_file" CHANGELOG.md
    
    log_success "Updated CHANGELOG.md"
}

# Create and push git tag
create_git_tag() {
    local version="$1"
    local tag_name="v$version"
    
    if [ "$DRY_RUN" = true ]; then
        log_info "Would create git tag: $tag_name"
        log_info "Would push tag to origin"
        return 0
    fi
    
    # Create annotated tag
    git tag -a "$tag_name" -m "Release $version"
    log_success "Created git tag: $tag_name"
    
    # Push tag to origin
    git push origin "$tag_name"
    log_success "Pushed tag to origin"
}

# Run pre-release checks
run_pre_release_checks() {
    log_info "Running pre-release checks..."
    
    # Check if on main branch
    local current_branch=$(git rev-parse --abbrev-ref HEAD)
    if [ "$current_branch" != "master" ] && [ "$current_branch" != "main" ]; then
        log_warning "Not on main/master branch (current: $current_branch)"
        if [ "$FORCE" = false ]; then
            log_error "Use --force to release from non-main branch"
            exit 1
        fi
    fi
    
    # Check if all tests pass
    log_info "Running tests..."
    if [ "$DRY_RUN" = false ]; then
        if ! ./test.sh; then
            log_error "Tests failed"
            exit 1
        fi
    else
        log_info "Would run tests"
    fi
    
    # Check if build succeeds
    log_info "Testing build..."
    if [ "$DRY_RUN" = false ]; then
        if ! ./build.sh; then
            log_error "Build failed"
            exit 1
        fi
    else
        log_info "Would test build"
    fi
}

# Main release process
main() {
    log_info "Starting release process..."
    
    # Check git status
    check_git_status
    
    # Determine version
    if [ -z "$VERSION" ]; then
        VERSION=$(auto_increment_version)
        log_info "Auto-incremented version to: $VERSION"
    fi
    
    # Validate version format
    validate_version "$VERSION"
    
    # Check if version already exists
    local current_version=$(get_current_version)
    if [ "$current_version" = "$VERSION" ]; then
        log_error "Version $VERSION already exists"
        exit 1
    fi
    
    # Check if tag already exists
    if git rev-parse "v$VERSION" >/dev/null 2>&1; then
        log_error "Tag v$VERSION already exists"
        exit 1
    fi
    
    log_info "Preparing release $VERSION"
    
    # Run pre-release checks
    run_pre_release_checks
    
    # Update version file
    update_version_file "$VERSION"
    
    # Generate changelog
    generate_changelog "$VERSION"
    
    # Commit changes
    if [ "$DRY_RUN" = false ]; then
        git add VERSION.TXT CHANGELOG.md
        git commit -m "Release $VERSION"
        log_success "Committed release changes"
    else
        log_info "Would commit release changes"
    fi
    
    # Create and push tag
    create_git_tag "$VERSION"
    
    if [ "$DRY_RUN" = false ]; then
        log_success "Release $VERSION completed!"
        log_info "GitHub Actions will now build and publish the release"
        log_info "Monitor the progress at: https://github.com/twardoch/imagebatchprocessor/actions"
    else
        log_info "Dry run completed - no changes made"
    fi
}

# Run main function
main "$@"