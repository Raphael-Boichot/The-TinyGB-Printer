Get-ChildItem -Path $driveLetter -Recurse | ForEach-Object {
    $filePath = $_.FullName
    Write-Host "Processing file: $filePath"  # Log the file being processed

    $attempts = 0
    $success = $false

    while ($attempts -lt $maxRetries -and -not $success) {
        try {
            # Remove the "ReadOnly" attribute if it is set
            if ($_.Attributes -band [System.IO.FileAttributes]::ReadOnly) {
                Write-Host "Removing Read-Only attribute from: $filePath"
                $_.Attributes = $_.Attributes -band -[System.IO.FileAttributes]::ReadOnly
            }

            # Set the date/time properties for each file/folder
            Set-ItemProperty -Path $filePath -Name CreationTime -Value $currentDateTime
            Set-ItemProperty -Path $filePath -Name LastAccessTime -Value $currentDateTime
            Set-ItemProperty -Path $filePath -Name LastWriteTime -Value $currentDateTime
            $success = $true # If successful, break the loop
        } catch {
            # If an error occurs, log it and retry
            Write-Host "Error accessing file: $filePath. Retrying..."
            $attempts++
            Start-Sleep -Seconds $retryDelay
        }
    }

    if (-not $success) {
        Write-Host "Failed to update timestamps for file: $filePath after $maxRetries attempts."
    }
}