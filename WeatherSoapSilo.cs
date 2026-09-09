using System.ServiceModel;
using ServiceReference;

namespace AstroCampPlanner;

internal readonly struct WeatherBulletin
{
    public WeatherBulletin(double temperatureF, string conditions, int humidityPercent, double windMph)
    {
        TemperatureF = temperatureF;
        Conditions = conditions;
        HumidityPercent = humidityPercent;
        WindMph = windMph;
    }

    public double TemperatureF { get; }
    public string Conditions { get; }
    public int HumidityPercent { get; }
    public double WindMph { get; }
}

internal static class WeatherSoapSilo
{
    private const int Port = 8080;
    private static readonly string ServerHome = EnvironmentPaths.Resolve(
        "ASTROCAMPPLANNER_WEATHER_SERVER_HOME",
        Path.Combine(ProjectLocator.Root, "TestServers", "WeatherSoapServer"));
    private static readonly string ServerExecutable = Path.Combine(ServerHome, "server", "weather_server");
    private static readonly string ServerWorkingDirectory = Path.Combine(ServerHome, "server");

    public static WeatherBulletin GetBulletin(string city)
    {
        ProcessSupervisor.EnsureListening(Port, ServerExecutable, ServerWorkingDirectory, Port.ToString());

        WeatherSoapClient client = new WeatherSoapClient();
        try
        {
            client.Open();

            GetWeatherResponse response = client.GetWeatherAsync(city).GetAwaiter().GetResult();
            WeatherData data = response.Body.GetWeatherResult;

            client.Close();

            return new WeatherBulletin(data.TemperatureF, data.Conditions, data.HumidityPercent, data.WindMph);
        }
        catch (FaultException faultException)
        {
            client.Abort();
            throw new InvalidOperationException("The weather bureau's SOAP service returned a fault: " + faultException.Message, faultException);
        }
        catch (CommunicationException communicationException)
        {
            client.Abort();
            throw new InvalidOperationException("Could not communicate with the weather bureau's SOAP service: " + communicationException.Message, communicationException);
        }
        catch (TimeoutException timeoutException)
        {
            client.Abort();
            throw new InvalidOperationException("The weather bureau's SOAP service did not respond in time.", timeoutException);
        }
    }
}
