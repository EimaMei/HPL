Param (
  $path = "C:\Program Files (x86)\HPL" # Option to change the path of where HPL will be installed.
)
$curPath = $PSScriptRoot # Get the current path


if (-Not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) { # If the user isn't running this in admin, reopen Powershell in admin and ask for admin permissions.
    $arguments = "& '" +$myinvocation.mycommand.definition + "'"
    Start-Process powershell -Verb runAs -ArgumentList $arguments
    Set-Location $curPath
    Break
}

$isAdmin = ([Security.Principal.WindowsPrincipal] `
  [Security.Principal.WindowsIdentity]::GetCurrent() `
).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)



If (-Not $isAdmin) { # Just in case we check if we DON'T have admin permissions.
    Write-Host "Please run this installer with administrator permissions to set the environment variable for HPL."
    Write-Host -NoNewLine 'Press any key to continue...';
    $null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown');
    Exit
}


if (Test-Path $path) { # If HPL was already installed, delete everything.
    Remove-Item $path -Recurse
}


New-Item -Path $path -ItemType Directory # Create the folder
Move-Item $curPath\hpl.exe $path # Move hpl.exe and core
Move-Item $curPath\core $path
Copy-Item $curPath\LICENSE $path # Copy LICENSE and examples
Copy-Item $curPath\examples $path


$ENVPATH = [Environment]::GetEnvironmentVariable("PATH", "Machine") # Get the environment variable path.
if ($ENVPATH -notlike "*" + $path + "*") { # Check if HPL isn't already in the variable.
    [Environment]::SetEnvironmentVariable("PATH", "$ENVPATH;$path", "Machine") # If not, add HPL to the environment variable.
}