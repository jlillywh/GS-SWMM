# GitHub Release Checklist - v4.1

Use this checklist to prepare and publish the v4.1 release to GitHub.

## Pre-Release Verification

### Code Changes
- [x] Version number updated in `SwmmGoldSimBridge.cpp` (4.0 → 4.1)
- [x] Version comment updated in XF_REP_VERSION case
- [x] README.md updated with new version number
- [x] README.md updated with "Known Limitations" section
- [x] Changelog expanded in README.md
- [x] Release notes created (`RELEASE_NOTES_v4.1.md`)

### Build Verification
- [ ] Clean build completed successfully
  ```batch
  # In Visual Studio
  Build → Clean Solution
  Build → Rebuild Solution
  ```
- [ ] Release configuration (x64) builds without errors
- [ ] Output DLL created: `x64/Release/GSswmm.dll`
- [ ] DLL version verified (check properties or test)

### Testing
- [ ] All unit tests pass
  ```batch
  cd tests
  run_all_tests.bat
  ```
- [ ] Integration test passes
  ```batch
  cd tests
  .\Run-IntegrationTest.ps1
  ```
- [ ] Manual smoke test with GoldSim (if available)

### Documentation Review
- [ ] README.md renders correctly on GitHub
- [ ] All links in README.md work
- [ ] Code examples are accurate
- [ ] Troubleshooting section is up to date
- [ ] API reference matches implementation

## GitHub Repository Preparation

### Commit Changes
```bash
git status
git add SwmmGoldSimBridge.cpp
git add README.md
git add RELEASE_NOTES_v4.1.md
git add GITHUB_RELEASE_CHECKLIST.md
git commit -m "Release v4.1: Document water quality API limitation"
```

### Tag the Release
```bash
git tag -a v4.1 -m "Version 4.1 - Water quality limitation documentation"
git push origin main
git push origin v4.1
```

## Create GitHub Release

### Navigate to Releases
1. Go to your GitHub repository
2. Click "Releases" in the right sidebar
3. Click "Draft a new release"

### Release Configuration

**Tag**: `v4.1`  
**Release Title**: `GSswmm Bridge v4.1 - Water Quality Limitation Documentation`

**Description** (copy this):

```markdown
## 🔍 Documentation Update Release

Version 4.1 clarifies an important limitation of the SWMM5 API regarding water quality data access during live simulation.

### 📋 What's New

- **Water Quality Limitation Documented**: Added comprehensive explanation that pollutant concentrations (TSS, BOD, etc.) are not accessible during runtime
- **API Investigation Results**: Documented which properties ARE available (hydraulic: flow, depth, volume)
- **Workaround Guidance**: Provided instructions for users who need water quality data
- **Expanded Changelog**: Full version history from 1.0 to 4.1

### ⚠️ Important Information

**The SWMM5 API does not expose water quality data during live simulation.** This is a fundamental limitation of the EPA SWMM engine, not the bridge implementation.

**What you CAN access in real-time:**
- ✅ Flow rates (links and nodes)
- ✅ Water depths
- ✅ Storage volumes
- ✅ Hydraulic properties

**What you CANNOT access in real-time:**
- ❌ Pollutant concentrations (TSS, BOD, etc.)
- ❌ Node quality values
- ❌ Link quality values

### 🔄 Breaking Changes

**None** - This is a documentation-only release. All existing models continue to work exactly as before.

### 📦 Installation

1. Download `GSswmm.dll` from the assets below
2. Copy to your GoldSim model directory
3. Follow the Quick Start guide in README.md

### 📚 Documentation

- [README.md](README.md) - Complete user guide
- [RELEASE_NOTES_v4.1.md](RELEASE_NOTES_v4.1.md) - Detailed release notes
- [Known Limitations](README.md#known-limitations) - Water quality limitation details

### 🧪 Testing

All tests pass:
- ✅ Unit tests (lifecycle, error handling, validation)
- ✅ Integration tests (treatment train model)
- ✅ Property-based tests

### 📋 Full Changelog

See [RELEASE_NOTES_v4.1.md](RELEASE_NOTES_v4.1.md) for complete details.

**Full Changelog**: v4.0...v4.1
```

### Attach Release Assets

Upload these files as release assets:

1. **GSswmm.dll** (from `x64/Release/`)
   - Description: "Main bridge DLL (x64 Release build)"

2. **swmm5.dll** (from project root)
   - Description: "SWMM5 engine DLL (required dependency)"

3. **RELEASE_NOTES_v4.1.md**
   - Description: "Detailed release notes"

4. **README.md**
   - Description: "Complete user guide"

### Release Options

- [ ] Set as the latest release: **YES**
- [ ] Set as a pre-release: **NO**
- [ ] Create a discussion for this release: **Optional**

### Publish

- [ ] Click "Publish release"

## Post-Release Tasks

### Verification
- [ ] Release appears on GitHub releases page
- [ ] Assets are downloadable
- [ ] Tag is visible in repository
- [ ] README renders correctly on main page

### Communication (Optional)
- [ ] Announce on project discussion board
- [ ] Update any external documentation
- [ ] Notify users who requested water quality features

### Archive
- [ ] Move this checklist to `docs/releases/` folder
- [ ] Update project roadmap if applicable

## Rollback Plan (If Needed)

If issues are discovered after release:

1. **Delete the release** (not the tag)
2. **Fix the issues**
3. **Create v4.1.1** with fixes
4. **Document what was fixed**

## Notes

- This is a documentation-only release
- No code functionality changed
- All existing GoldSim models remain compatible
- Users do not need to update unless they want the improved documentation

---

**Release Manager**: _________________  
**Date Completed**: _________________  
**GitHub Release URL**: _________________
