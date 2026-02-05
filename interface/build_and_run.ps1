$env:PATH = "C:\Qt\Tools\mingw1120_64\bin;C:\Qt\6.7.3\mingw_64\bin;$env:PATH"
$buildDir = "build"

Get-Process interface -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 1

if (Test-Path $buildDir) {
    Remove-Item -Recurse -Force $buildDir
}

cmake -G "MinGW Makefiles" -S . -B $buildDir -DCMAKE_PREFIX_PATH="C:/Qt/6.7.3/mingw_64"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build $buildDir
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Copy-Item "style.qss" -Destination "$buildDir/style.qss" -Force

Write-Host "Build successful. Running application..."
& "$buildDir\interface.exe"
