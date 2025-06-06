name: Build Windows Standalone

on:
  workflow_dispatch:
  push:
    branches: [ main, master ]

jobs:
  build-standalone:
    runs-on: windows-latest
    
    steps:
    - name: Checkout repository
      uses: actions/checkout@v4
      
    - name: Download JUCE 6.1.6
      run: |
        Write-Host "Downloading JUCE 6.1.6 for standalone build..."
        Invoke-WebRequest -Uri "https://github.com/juce-framework/JUCE/archive/refs/tags/6.1.6.zip" -OutFile "juce.zip"
        Expand-Archive juce.zip -DestinationPath . -Force
        Move-Item JUCE-6.1.6 JUCE
        
    - name: Setup MSVC
      uses: microsoft/setup-msbuild@v2
      
    - name: Configure CMake for Standalone
      run: |
        cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
        
    - name: Build Standalone Application
      run: |
        cmake --build build --config Release --target FootstepDetector_Standalone
        
    - name: Verify Standalone Build
      run: |
        $standalonePath = "build\FootstepDetector_artefacts\Release\Standalone\FootstepDetector.exe"
        if (Test-Path $standalonePath) {
          Write-Host "✅ Standalone .exe built successfully"
          Get-Item $standalonePath | Select-Object Name, Length, LastWriteTime
        } else {
          Write-Host "❌ Standalone build failed"
          Get-ChildItem build -Recurse -Name "*.exe"
          exit 1
        }
        
    - name: Upload Windows Standalone EXE
      uses: actions/upload-artifact@v4
      with:
        name: FootstepDetector-Windows-Standalone-EXE
        path: build/FootstepDetector_artefacts/Release/Standalone/FootstepDetector.exe
        
    - name: Create Standalone Package
      run: |
        mkdir FootstepDetector_Standalone_Windows
        copy build\FootstepDetector_artefacts\Release\Standalone\FootstepDetector.exe FootstepDetector_Standalone_Windows\
        echo "FootstepDetector Standalone Application - Windows x64" > FootstepDetector_Standalone_Windows\README.txt
        echo "" >> FootstepDetector_Standalone_Windows\README.txt
        echo "Usage:" >> FootstepDetector_Standalone_Windows\README.txt
        echo "1. Double-click FootstepDetector.exe to run" >> FootstepDetector_Standalone_Windows\README.txt
        echo "2. Select your audio input/output devices" >> FootstepDetector_Standalone_Windows\README.txt
        echo "3. Adjust Sensitivity, Gain, and Bypass controls" >> FootstepDetector_Standalone_Windows\README.txt
        echo "4. Real-time footstep detection and enhancement" >> FootstepDetector_Standalone_Windows\README.txt
        echo "" >> FootstepDetector_Standalone_Windows\README.txt
        echo "No installation required - runs directly!" >> FootstepDetector_Standalone_Windows\README.txt
        
    - name: Upload Complete Standalone Package
      uses: actions/upload-artifact@v4
      with:
        name: FootstepDetector-Windows-Standalone-Complete
        path: FootstepDetector_Standalone_Windows/