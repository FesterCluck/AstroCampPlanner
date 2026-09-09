using AstroCampPlanner;
using AstroClient;
using ParksRpcClient;

Environment.SetEnvironmentVariable("TERM", "linux");

Console.WriteLine("=================================================");
Console.WriteLine("  AstroCampPlanner -- plan your stargazing trip");
Console.WriteLine("=================================================");
Console.WriteLine();

Console.WriteLine("Fetching the park roster (ONC RPC)...");
IReadOnlyList<Park> parks;
try
{
    parks = ParksRpcSilo.ListAllParks();
}
catch (Exception ex)
{
    Console.WriteLine($"Could not reach the parks RPC service: {ex.Message}");
    return 1;
}

Console.WriteLine();
Console.WriteLine("Choose a national park:");
for (int i = 0; i < parks.Count; i++)
{
    Console.WriteLine($"  {i + 1,2}. {parks[i].Name} ({parks[i].State})");
}
Park park = parks[PromptIndex("Park #", parks.Count)];

Console.WriteLine();
Console.WriteLine("Choose a constellation:");
IReadOnlyList<string> constellations = AstroRestSilo.Constellations;
for (int i = 0; i < constellations.Count; i++)
{
    Console.WriteLine($"  {i + 1,2}. {constellations[i]}");
}
string constellation = constellations[PromptIndex("Constellation #", constellations.Count)];

Console.WriteLine();
Console.WriteLine($"Calling the transit-time service (REST) for {constellation} over {park.Name}...");
TransitResult transit;
try
{
    transit = await AstroRestSilo.GetTransitAsync(park.Latitude, park.Longitude, constellation);
}
catch (Exception ex)
{
    Console.WriteLine($"Could not reach the astronomical transit service: {ex.Message}");
    return 1;
}

Console.WriteLine();
Console.WriteLine($"Dialing the weather bureau (SOAP) for {park.Name}...");
WeatherBulletin weather;
try
{
    weather = WeatherSoapSilo.GetBulletin(park.Name);
}
catch (Exception ex)
{
    Console.WriteLine($"Could not reach the weather bureau: {ex.Message}");
    return 1;
}

Console.WriteLine();
Console.WriteLine("=================================================");
Console.WriteLine($"  {constellation} is highest over {park.Name}, {park.State}");
Console.WriteLine($"  on {transit.TransitTimeUtc:yyyy-MM-dd} at {transit.TransitTimeUtc:HH:mm} UTC");
Console.WriteLine($"  (max altitude {transit.MaxAltitudeDeg:F1} deg, visibility: {transit.Visibility})");
Console.WriteLine();
Console.WriteLine("  WEATHER BULLETIN:");
Console.WriteLine($"    {weather.TemperatureF:F0}F, {weather.Conditions}, {weather.HumidityPercent}% humidity, {weather.WindMph:F1} mph wind");
Console.WriteLine("=================================================");
Console.WriteLine();

Console.WriteLine("This planning service costs $5.00. Let's process your payment over our dial-up terminal.");
Console.WriteLine("(Demo cards: 4242 4242 4242 4242 = approved, 4000000000000002 = declined; expiry 01/99, cvv 123)");
Console.Write("Card number: ");
string card = Console.ReadLine() ?? string.Empty;
Console.Write("Expiry (MM/YY): ");
string expiry = Console.ReadLine() ?? string.Empty;
Console.Write("CVV: ");
string cvv = Console.ReadLine() ?? string.Empty;

Console.WriteLine();
Console.WriteLine("Dialing out...");
AuthResult auth = DialupPaymentSilo.ChargeCard(card, expiry, cvv, "5.00");
Console.WriteLine(auth.Transcript);

switch (auth.Outcome)
{
    case AuthOutcome.Approved:
        Console.WriteLine("Payment APPROVED. Thank you -- clear skies!");
        return 0;
    case AuthOutcome.Declined:
        Console.WriteLine("Payment DECLINED. Your itinerary was not saved.");
        return 1;
    case AuthOutcome.RejectedLocally:
        Console.WriteLine("That card is not accepted by this terminal.");
        return 1;
    case AuthOutcome.NoCarrier:
        Console.WriteLine("NO CARRIER -- could not reach the card processor.");
        return 1;
    default:
        Console.WriteLine("An error occurred while processing payment.");
        return 1;
}

int PromptIndex(string label, int count)
{
    while (true)
    {
        Console.Write($"{label} (1-{count}): ");
        string? input = Console.ReadLine();
        if (int.TryParse(input, out int n) && n >= 1 && n <= count) return n - 1;
        Console.WriteLine("Invalid selection, try again.");
    }
}
