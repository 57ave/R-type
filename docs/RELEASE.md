# Release Workflow

This repository uses an automated release workflow to build and publish R-Type binaries for Windows and Linux.

## How to Create a Release

### Option 1: Manual Trigger (Recommended)

1. Go to **Actions** tab in GitHub
2. Select **Release** workflow
3. Click **Run workflow**
4. Choose version bump type:
   - `patch` - Bug fixes (1.0.0 → 1.0.1)
   - `minor` - New features (1.0.0 → 1.1.0)
   - `major` - Breaking changes (1.0.0 → 2.0.0)
5. Click **Run workflow**

The workflow will:
- Increment version in `VERSION` file
- Build for Linux and Windows
- Package binaries with all assets
- Create GitHub release with downloads
- Push version tag to repository

### Option 2: Tag Push

Push a tag matching `v*.*.*` pattern:
```bash
git tag v1.0.0
git push origin v1.0.0
```

## Release Artifacts

Each release includes:

### Windows (`r-type-windows-v{version}.zip`)
- `r-type_server.exe` - Game server
- `r-type_client.exe` - Game client  
- `r-type_game.exe` - Standalone game
- All required `.dll` files
- Complete `assets/` folder

### Linux (`r-type-linux-v{version}.tar.gz`)
- `r-type_server` - Game server
- `r-type_client` - Game client
- `r-type_game` - Standalone game
- All required `.so` files
- Complete `assets/` folder

## Version Management

Current version is stored in the `VERSION` file at the project root.

Format: `MAJOR.MINOR.PATCH`

The workflow automatically:
1. Reads current version
2. Increments based on bump type
3. Updates `VERSION` file
4. Commits and tags the new version
5. Creates release with the new version number

## Testing a Release

After downloading:

**Windows:**
```cmd
unzip r-type-windows-v1.0.0.zip
cd r-type-windows-v1.0.0\bin
r-type_game.exe
```

**Linux:**
```bash
tar -xzf r-type-linux-v1.0.0.tar.gz
cd r-type-linux-v1.0.0/bin
./r-type_game
```

## Troubleshooting

**Workflow fails on version bump:**
- Ensure `VERSION` file exists and contains valid `MAJOR.MINOR.PATCH` format
- Check that you have write permissions to the repository

**Build fails:**
- Check the compilation workflow first - it should pass before releasing
- Review build logs in the Actions tab

**Release not created:**
- Verify GitHub token has release creation permissions
- Check that tag doesn't already exist
