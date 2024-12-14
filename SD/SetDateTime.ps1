# PowerShell Script to Update File Timestamps to Current System Time
# Script should be placed at the root of the SD card

# Get the current system time
$currentDateTime = Get-Date

# Automatically get the drive letter of the script's location
$scriptPath = $MyInvocation.MyCommand.Path
$driveLetter = [System.IO.Path]::GetPathRoot($scriptPath)

# Ensure we detected the drive letter
if ($driveLetter) {
    Write-Host "Detected SD card at drive letter: $driveLetter"
    
    # Function to update file timestamps
    function Update-FileTimestamps {
        param (
            [string]$filePath
        )

        # Set the file timestamps to current system time
        try {
            Write-Host "Updating timestamps for: $filePath"
            Set-ItemProperty -Path $filePath -Name "CreationTime" -Value $currentDateTime
            Set-ItemProperty -Path $filePath -Name "LastAccessTime" -Value $currentDateTime
            Set-ItemProperty -Path $filePath -Name "LastWriteTime" -Value $currentDateTime
        } catch {
            Write-Host "Failed to update timestamps for $filePath. Error: $_"
        }
    }

    # Recursively get all files in the root folder and subfolders
    $rootFolder = "$driveLetter"  # This is where the script is located (the SD card root)
    $files = Get-ChildItem -Path $rootFolder -Recurse -File

    # Update timestamps for each file
    foreach ($file in $files) {
        Update-FileTimestamps -filePath $file.FullName
    }

    Write-Host "Timestamps update complete."
} else {
    Write-Host "Unable to detect the drive letter. Ensure this script is placed at the root of the SD card."
}