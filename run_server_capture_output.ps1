# PowerShell script for building, deploying, and testing HLDS with 60-second timeout
Write-Host "Capturing HLDS.EXE output for debugging..." -ForegroundColor Green
Set-Location "C:\Program Files (x86)\Steam\steamapps\common\msrebirth"

# Kill any existing processes
Write-Host "Killing existing HLDS processes..." -ForegroundColor Yellow
Get-Process -Name "hlds" -ErrorAction SilentlyContinue | Stop-Process -Force

# Go back to project directory to compile and deploy
Set-Location "C:\Users\Rhetoric\Documents\Github\MasterSwordRebirth"

# Remove build folder and utils/build folder contents
Write-Host "Removing build folder contents..." -ForegroundColor Yellow
Remove-Item "build" -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "Removing utils/build folder contents..." -ForegroundColor Yellow
Remove-Item "utils\build" -Recurse -Force -ErrorAction SilentlyContinue

# Remove previous server_output.txt and server_error.txt
Write-Host "Removing previous server_output.txt and server_error.txt..." -ForegroundColor Yellow
Remove-Item "server_output.txt" -ErrorAction SilentlyContinue
Remove-Item "server_error.txt" -ErrorAction SilentlyContinue

# Build project files
Write-Host "Building project files..." -ForegroundColor Cyan
& cmake -S . -B ./build -A Win32
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed for main project with exit code $LASTEXITCODE" -ForegroundColor Red
    exit 1
}
& cmake -S utils -B utils/build -A Win32
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed for utils with exit code $LASTEXITCODE" -ForegroundColor Red
    exit 1
}

# Clear bin folder contents
Write-Host "Clearing bin folder contents..." -ForegroundColor Yellow
Remove-Item "bins\debug" -Recurse -Force -ErrorAction SilentlyContinue

# Build server DLL and scriptpack
Write-Host "Building server DLL..." -ForegroundColor Cyan
& cmake --build build --config Debug --target server
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit 1
}

Write-Host "Building scriptpack..." -ForegroundColor Cyan
& cmake --build build --config Debug --target scriptpack
if ($LASTEXITCODE -ne 0) {
    Write-Host "Scriptpack build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit 1
}

# Deploy DLL and scriptpack
Write-Host "Deploying server DLL..." -ForegroundColor Cyan
Copy-Item "bins\debug\ms.dll" "C:\Program Files (x86)\Steam\steamapps\common\msrebirth\msr\dlls\ms.dll" -Force

Write-Host "Deploying updated scriptpack.exe..." -ForegroundColor Cyan
Copy-Item "bins\debug\scriptpack.exe" "MSCScripts\scriptpack.exe" -Force

# Pack and deploy scripts
Write-Host "Packing scripts..." -ForegroundColor Cyan
Set-Location "MSCScripts"

# Use echo to automatically send Enter to bypass the pause
Set-Location "scripts"
Write-Host "Setting time stamp." -ForegroundColor Yellow
"{" | Out-File -FilePath "beta_date.script" -Encoding ASCII
"`tconst BETA_TIMESTAMP `"CANARY: $(Get-Date -Format 'MM/dd/yyyy HH:mm:ss')`"" | Out-File -FilePath "beta_date.script" -Append -Encoding ASCII
"}" | Out-File -FilePath "beta_date.script" -Append -Encoding ASCII
Set-Location ".."

Write-Host "Compiling..." -ForegroundColor Yellow

$scriptpack = Start-Process -FilePath "./scriptpack.exe" -ArgumentList "-ef --angelscript-lint" -PassThru -NoNewWindow
$scriptpack.WaitForExit()
if ($scriptpack.ExitCode -ne 0) {
    Write-Host "Script packing failed with exit code $($scriptpack.ExitCode)" -ForegroundColor Red
    exit 1
}
Copy-Item "scripts.pak" "C:\Program Files (x86)\Steam\steamapps\common\msrebirth\msr\scripts.pak" -Force

# Go back to server directory
Set-Location "C:\Program Files (x86)\Steam\steamapps\common\msrebirth"

# Clear logs
Write-Host "Clearing ALL logs..." -ForegroundColor Yellow
Remove-Item "qconsole.log" -ErrorAction SilentlyContinue
Remove-Item "msr\log_msdll.log" -ErrorAction SilentlyContinue
Remove-Item "msr\logs\*.log" -ErrorAction SilentlyContinue
Remove-Item "server_debug_output.txt" -ErrorAction SilentlyContinue
Remove-Item "*.log" -ErrorAction SilentlyContinue
Remove-Item "msr\*.log" -ErrorAction SilentlyContinue
Write-Host "Logs cleared." -ForegroundColor Green

Write-Host "Starting server with debug output..." -ForegroundColor Green
Write-Host "Server will run for 60 seconds to test AngelScript..." -ForegroundColor Yellow

# Start server and capture output for 60 seconds
Write-Host "Starting HLDS for 60 seconds..." -ForegroundColor Cyan
$arguments = @(
    "-console",
    "-noipx", 
    "-condebug",
    "-insecure",
    "-num_edicts", "2048",
    "-heapsize", "131072",
    "-game", "msr",
    "+maxplayers", "10",
    "-port", "27016",
    "+map", "edana",
    "+log", "on",
    "+exec", "crashed.cfg"
)

Write-Host "Command: hlds.exe $($arguments -join ' ')" -ForegroundColor Gray

# Start the process and capture output
$hldsPath = Join-Path (Get-Location) "hlds.exe"
$process = Start-Process -FilePath $hldsPath -ArgumentList $arguments -PassThru -NoNewWindow -RedirectStandardOutput "server_output.txt" -RedirectStandardError "server_error.txt"

Write-Host "Server started (PID: $($process.Id)), waiting 60 seconds..." -ForegroundColor Green

# Wait 60 seconds
Start-Sleep -Seconds 60

# Stop the server
Write-Host "Stopping server after 60 seconds..." -ForegroundColor Yellow
try {
    Stop-Process -Id $process.Id -Force
    Write-Host "Server stopped successfully." -ForegroundColor Green
} catch {
    Write-Host "Server process already stopped or could not be stopped." -ForegroundColor Yellow
}

# Kill any remaining processes
Write-Host "Ensuring all HLDS processes are stopped..." -ForegroundColor Yellow
Get-Process -Name "hlds" -ErrorAction SilentlyContinue | Stop-Process -Force

Write-Host ""
Write-Host "Server test completed. Checking for generated logs..." -ForegroundColor Green
Write-Host ""

# Display captured output
Write-Host "=== SERVER STDOUT ===" -ForegroundColor Cyan
if (Test-Path "server_output.txt") {
    Get-Content "server_output.txt"
} else {
    Write-Host "No server_output.txt found" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== SERVER STDERR ===" -ForegroundColor Cyan
if (Test-Path "server_error.txt") {
    Get-Content "server_error.txt"
} else {
    Write-Host "No server_error.txt found" -ForegroundColor Yellow
}

Read-Host "Press Enter to continue..."