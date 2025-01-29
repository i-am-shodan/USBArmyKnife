# Define the COM port and baud rate
$portName = "COM4"
$baudRate = 115200

# Try to open the COM port
while ($true)
{
    try {
        $port = new-Object System.IO.Ports.SerialPort $portName, $baudRate, None, 8, one
        $port.Open()
        $port.ReadTimeout = -1 # Set read timeout to 500 milliseconds
        $port.WriteTimeout = -1
        $port.DtrEnable = $true
        $port.RtsEnable = $true
        break
    } catch {
        Write-Host "Error: Unable to open COM port $portName"
        Start-Sleep -Seconds 1
    }
}

# Function to send a TLV encoded message
function Send-TLV {
    param (
        [System.IO.Ports.SerialPort]$port,
        [byte]$tag,
        [string]$value
    )

    # Convert the value to bytes
    $valueBytes = [System.Text.Encoding]::ASCII.GetBytes($value)
    $length = $valueBytes.Length

    # Convert the length to 4 bytes
    $lengthBytes = [BitConverter]::GetBytes($length)

    # Create the TLV message
    $message = New-Object byte[] (1 + 4 + $length)
    $message[0] = $tag
    [Array]::Copy($lengthBytes, 0, $message, 1, 4)
    [Array]::Copy($valueBytes, 0, $message, 5, $length)

    # Send the TLV message
    $port.Write($message, 0, $message.Length)
}

# Send the TLV message with tag 0x8 and value "debugscript"
Send-TLV -port $port -tag 0x8 -value "debugscript"

# Function to read TLV data
function Read-TLV {
    param (
        [System.IO.Ports.SerialPort]$port
    )

    while ($port.IsOpen) {
        try {
            # Read the tag (1 byte)
            $tag = $port.ReadByte()

            # Read the length (4 bytes)
            $lengthBytes = New-Object byte[] 4
            $port.Read($lengthBytes, 0, 4) > $null
            $length = [BitConverter]::ToInt32($lengthBytes, 0)

            # Read the value (length bytes)
            $valueBytes = New-Object byte[] $length
            $port.Read($valueBytes, 0, $length) > $null

            if ($tag -ne 0x2) {
                continue
            }

            $value = [System.Text.Encoding]::ASCII.GetString($valueBytes)

            # Print the ASCII string value
            Write-Host "$value"
        } catch {
            Write-Host "Error: Failed to read from COM port"
            $port.Close()
            exit
        }
    }
}

# Start reading TLV data
Read-TLV -port $port

# Close the COM port when done
$port.Close()