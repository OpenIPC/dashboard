#!/bin/bash
# VMS Dashboard Build Script for Linux/macOS
# Downloads MediaMTX binaries and builds the application

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Default values
PLATFORM=""
DEBUG=false
DOWNLOAD_ONLY=false
HELP=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --platform)
            PLATFORM="$2"
            shift 2
            ;;
        --debug)
            DEBUG=true
            shift
            ;;
        --download-only)
            DOWNLOAD_ONLY=true
            shift
            ;;
        --help|-h)
            HELP=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

show_help() {
    echo -e "${GREEN}VMS Dashboard Build Script${NC}"
    echo -e "${GREEN}=========================${NC}"
    echo ""
    echo -e "${YELLOW}Usage:${NC}"
    echo "  ./tools/build.sh                      # Build for current platform"
    echo "  ./tools/build.sh --platform windows   # Build for Windows"
    echo "  ./tools/build.sh --platform linux     # Build for Linux"
    echo "  ./tools/build.sh --platform macos     # Build for macOS"
    echo "  ./tools/build.sh --debug              # Build in debug mode"
    echo "  ./tools/build.sh --download-only      # Only download binaries"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo "  npm run build-release                 # Build release"
    echo "  npm run build-debug                   # Build debug"
    echo "  npm run download-mediamtx             # Download MediaMTX"
    echo "  npm run download-gstreamer            # Download optional GStreamer bundle"
    echo ""
}

check_command() {
    if command -v "$1" &> /dev/null; then
        echo -e "${GREEN}✅ $1 found: $(command -v $1)${NC}"
        return 0
    else
        echo -e "${RED}❌ $1 not found${NC}"
        return 1
    fi
}

check_prerequisites() {
    echo -e "${YELLOW}🔍 Checking prerequisites...${NC}"
    
    local python_ok=true
    local node_ok=true
    local rust_ok=true
    
    if ! check_command python3 && ! check_command python; then
        echo -e "${RED}   Please install Python 3.6+${NC}"
        python_ok=false
    fi
    
    if ! check_command node; then
        echo -e "${RED}   Please install Node.js 18+${NC}"
        echo -e "${YELLOW}   Download from: https://nodejs.org/${NC}"
        node_ok=false
    fi
    
    if ! check_command rustc; then
        echo -e "${RED}   Please install Rust${NC}"
        echo -e "${YELLOW}   Install from: https://rustup.rs/${NC}"
        rust_ok=false
    fi
    
    if [[ "$python_ok" == false || "$node_ok" == false || "$rust_ok" == false ]]; then
        echo ""
        echo -e "${RED}❌ Missing prerequisites. Please install the required tools.${NC}"
        exit 1
    fi
}

main() {
    if [[ "$HELP" == true ]]; then
        show_help
        return
    fi

    echo -e "${CYAN}🚀 VMS Dashboard Build Script${NC}"
    echo -e "${CYAN}=============================${NC}"
    echo ""

    # Check prerequisites
    check_prerequisites

    echo ""
    echo -e "${YELLOW}🔧 Starting build process...${NC}"

    # Prepare Python command
    local python_cmd="python3"
    if ! command -v python3 &> /dev/null; then
        python_cmd="python"
    fi

    local args=("tools/build.py")
    
    if [[ -n "$PLATFORM" ]]; then
        args+=("--platform" "$PLATFORM")
    fi
    
    if [[ "$DEBUG" == true ]]; then
        args+=("--debug")
    fi
    
    if [[ "$DOWNLOAD_ONLY" == true ]]; then
        args+=("--download-only")
    fi

    # Run Python build script
    if "$python_cmd" "${args[@]}"; then
        echo ""
        echo -e "${GREEN}🎉 Build completed successfully!${NC}"
        echo ""
        echo -e "${YELLOW}📁 Build outputs can be found in:${NC}"
        echo -e "   src-tauri/target/release/bundle/"
    else
        echo ""
        echo -e "${RED}❌ Build failed${NC}"
        exit 1
    fi
}

# Run main function
main