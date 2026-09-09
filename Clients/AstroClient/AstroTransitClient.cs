using System.Text.Json;

namespace AstroClient;

public sealed class AstroTransitClient : IDisposable
{
    private readonly HttpClient _http;
    private static readonly JsonSerializerOptions JsonOptions = new() { PropertyNameCaseInsensitive = true };

    public AstroTransitClient(string baseUrl)
    {
        _http = new HttpClient { BaseAddress = new Uri(baseUrl) };
    }

    public async Task<TransitResult> GetTransitAsync(double latitude, double longitude, string constellation, CancellationToken ct = default)
    {
        string url = $"/transit?lat={Uri.EscapeDataString(latitude.ToString("R"))}" +
                     $"&lng={Uri.EscapeDataString(longitude.ToString("R"))}" +
                     $"&constellation={Uri.EscapeDataString(constellation)}";

        HttpResponseMessage resp = await _http.GetAsync(url, ct);
        string body = await resp.Content.ReadAsStringAsync(ct);

        if (!resp.IsSuccessStatusCode)
        {
            string message = TryReadError(body) ?? $"HTTP {(int)resp.StatusCode}";
            throw new AstroClientException(message, (int)resp.StatusCode);
        }

        TransitResult? result = JsonSerializer.Deserialize<TransitResult>(body, JsonOptions);
        return result ?? throw new AstroClientException("the server returned an empty response");
    }

    private static string? TryReadError(string body)
    {
        try
        {
            using var doc = JsonDocument.Parse(body);
            return doc.RootElement.TryGetProperty("error", out JsonElement err) ? err.GetString() : null;
        }
        catch (JsonException)
        {
            return null;
        }
    }

    public void Dispose() => _http.Dispose();
}
