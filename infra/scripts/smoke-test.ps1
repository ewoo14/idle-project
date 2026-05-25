$ErrorActionPreference = "Stop"

$BaseUrl = if ($env:BASE_URL) { $env:BASE_URL } else { "http://localhost:3000" }
$Stamp = [DateTimeOffset]::UtcNow.ToUnixTimeSeconds().ToString()
$Email = "smoke-$Stamp@idle.local"
$Password = "Password123!"
$Nickname = "smoke$($Stamp.Substring($Stamp.Length - 5))"

$Register = Invoke-RestMethod -Method Post -Uri "$BaseUrl/v1/auth/register" -ContentType "application/json" -Body (@{
  email = $Email
  password = $Password
  nickname = $Nickname
} | ConvertTo-Json)

$Headers = @{ Authorization = "Bearer $($Register.data.accessToken)" }

$Character = Invoke-RestMethod -Method Post -Uri "$BaseUrl/v1/characters" -Headers $Headers -ContentType "application/json" -Body (@{
  classId = 1
} | ConvertTo-Json)

Invoke-RestMethod -Method Put -Uri "$BaseUrl/v1/save" -Headers $Headers -ContentType "application/json" -Body (@{
  characterId = $Character.data.id
  version = 1
  payload = @{
    level = 1
    rebirthCount = 0
    maxEquipmentGrade = 0
  }
} | ConvertTo-Json -Depth 5) | Out-Null

Invoke-RestMethod -Method Get -Uri "$BaseUrl/v1/leaderboard/power?season=1&limit=10" | Out-Null
Write-Host "smoke test passed"
