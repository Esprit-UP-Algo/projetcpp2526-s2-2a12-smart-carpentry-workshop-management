# Build & Run (Windows — MinGW via Qt)

Quick steps to configure, build and run this project using the MinGW toolchain bundled with Qt.

Configure (use the MinGW compiler provided by your Qt installation):

```powershell
cmake -S . -B build -G "MinGW Makefiles" \
  -DCMAKE_C_COMPILER="C:/Qt/Tools/mingw1120_64/bin/gcc.exe" \
  -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1120_64/bin/g++.exe" \
  -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/mingw1120_64/bin/mingw32-make.exe" \
  -DCMAKE_PREFIX_PATH="C:/Qt/6.7.3/mingw_64/lib/cmake"
```

Build:

```powershell
cmake --build build --config Debug
```

Run (example paths):

```powershell
# If using a config: .\build\Debug\WoodFlow.exe
# Otherwise: .\build\WoodFlow.exe
```

Make PATH persistent (optional — adds Qt's MinGW bin to your user PATH):

```powershell
$new = [Environment]::GetEnvironmentVariable('Path','User')
if ($new -notmatch 'C:\\Qt\\Tools\\mingw1120_64\\bin') {
  [Environment]::SetEnvironmentVariable('Path', $new + ';C:\Qt\Tools\mingw1120_64\bin', 'User')
}
# Then restart your terminal/VS Code/Qt Creator to pick up the change.
```

Qt Creator notes:
- Open the folder and select `CMakeLists.txt`.
- In Tools → Options → Kits: ensure the Compiler points to `C:/Qt/Tools/mingw1120_64/bin/g++.exe` and the Qt version points to `C:/Qt/6.7.3/mingw_64`.

Files changed by the workspace helper:
- [.vscode/settings.json](.vscode/settings.json) (forced `MinGW Makefiles` generator)
