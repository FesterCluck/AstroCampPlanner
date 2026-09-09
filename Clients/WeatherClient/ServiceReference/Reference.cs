namespace ServiceReference
{
    using System.Runtime.Serialization;
    
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Tools.ServiceModel.Svcutil", "2.1.0")]
    [System.Runtime.Serialization.DataContractAttribute(Name="WeatherData", Namespace="http://example.com/weather")]
    public partial class WeatherData : object
    {
        
        private string CityField;
        
        private double TemperatureFField;
        
        private string ConditionsField;
        
        private int HumidityPercentField;
        
        private double WindMphField;
        
        [System.Runtime.Serialization.DataMemberAttribute(IsRequired=true, EmitDefaultValue=false)]
        public string City
        {
            get
            {
                return this.CityField;
            }
            set
            {
                this.CityField = value;
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute(IsRequired=true)]
        public double TemperatureF
        {
            get
            {
                return this.TemperatureFField;
            }
            set
            {
                this.TemperatureFField = value;
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute(IsRequired=true, EmitDefaultValue=false, Order=2)]
        public string Conditions
        {
            get
            {
                return this.ConditionsField;
            }
            set
            {
                this.ConditionsField = value;
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute(IsRequired=true, Order=3)]
        public int HumidityPercent
        {
            get
            {
                return this.HumidityPercentField;
            }
            set
            {
                this.HumidityPercentField = value;
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute(IsRequired=true, Order=4)]
        public double WindMph
        {
            get
            {
                return this.WindMphField;
            }
            set
            {
                this.WindMphField = value;
            }
        }
    }
    
    [System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Tools.ServiceModel.Svcutil", "2.1.0")]
    [System.ServiceModel.ServiceContractAttribute(Namespace="http://example.com/weather", ConfigurationName="ServiceReference.WeatherSoap")]
    public interface WeatherSoap
    {
        
        [System.ServiceModel.OperationContractAttribute(Action="http://example.com/weather/GetWeather", ReplyAction="*")]
        System.Threading.Tasks.Task<ServiceReference.GetWeatherResponse> GetWeatherAsync(ServiceReference.GetWeatherRequest request);
    }
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Tools.ServiceModel.Svcutil", "2.1.0")]
    [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Advanced)]
    [System.ServiceModel.MessageContractAttribute(IsWrapped=false)]
    public partial class GetWeatherRequest
    {
        
        [System.ServiceModel.MessageBodyMemberAttribute(Name="GetWeather", Namespace="http://example.com/weather", Order=0)]
        public ServiceReference.GetWeatherRequestBody Body;
        
        public GetWeatherRequest()
        {
        }
        
        public GetWeatherRequest(ServiceReference.GetWeatherRequestBody Body)
        {
            this.Body = Body;
        }
    }
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Tools.ServiceModel.Svcutil", "2.1.0")]
    [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Advanced)]
    [System.Runtime.Serialization.DataContractAttribute(Namespace="http://example.com/weather")]
    public partial class GetWeatherRequestBody
    {
        
        [System.Runtime.Serialization.DataMemberAttribute(EmitDefaultValue=false, Order=0)]
        public string city;
        
        public GetWeatherRequestBody()
        {
        }
        
        public GetWeatherRequestBody(string city)
        {
            this.city = city;
        }
    }
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Tools.ServiceModel.Svcutil", "2.1.0")]
    [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Advanced)]
    [System.ServiceModel.MessageContractAttribute(IsWrapped=false)]
    public partial class GetWeatherResponse
    {
        
        [System.ServiceModel.MessageBodyMemberAttribute(Name="GetWeatherResponse", Namespace="http://example.com/weather", Order=0)]
        public ServiceReference.GetWeatherResponseBody Body;
        
        public GetWeatherResponse()
        {
        }
        
        public GetWeatherResponse(ServiceReference.GetWeatherResponseBody Body)
        {
            this.Body = Body;
        }
    }
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Tools.ServiceModel.Svcutil", "2.1.0")]
    [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Advanced)]
    [System.Runtime.Serialization.DataContractAttribute(Namespace="http://example.com/weather")]
    public partial class GetWeatherResponseBody
    {
        
        [System.Runtime.Serialization.DataMemberAttribute(EmitDefaultValue=false, Order=0)]
        public ServiceReference.WeatherData GetWeatherResult;
        
        public GetWeatherResponseBody()
        {
        }
        
        public GetWeatherResponseBody(ServiceReference.WeatherData GetWeatherResult)
        {
            this.GetWeatherResult = GetWeatherResult;
        }
    }
    
    [System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Tools.ServiceModel.Svcutil", "2.1.0")]
    public interface WeatherSoapChannel : ServiceReference.WeatherSoap, System.ServiceModel.IClientChannel
    {
    }
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Tools.ServiceModel.Svcutil", "2.1.0")]
    public partial class WeatherSoapClient : System.ServiceModel.ClientBase<ServiceReference.WeatherSoap>, ServiceReference.WeatherSoap
    {
        
        static partial void ConfigureEndpoint(System.ServiceModel.Description.ServiceEndpoint serviceEndpoint, System.ServiceModel.Description.ClientCredentials clientCredentials);
        
        public WeatherSoapClient() : 
                base(WeatherSoapClient.GetDefaultBinding(), WeatherSoapClient.GetDefaultEndpointAddress())
        {
            this.Endpoint.Name = EndpointConfiguration.WeatherSoap.ToString();
            ConfigureEndpoint(this.Endpoint, this.ClientCredentials);
        }
        
        public WeatherSoapClient(EndpointConfiguration endpointConfiguration) : 
                base(WeatherSoapClient.GetBindingForEndpoint(endpointConfiguration), WeatherSoapClient.GetEndpointAddress(endpointConfiguration))
        {
            this.Endpoint.Name = endpointConfiguration.ToString();
            ConfigureEndpoint(this.Endpoint, this.ClientCredentials);
        }
        
        public WeatherSoapClient(EndpointConfiguration endpointConfiguration, string remoteAddress) : 
                base(WeatherSoapClient.GetBindingForEndpoint(endpointConfiguration), new System.ServiceModel.EndpointAddress(remoteAddress))
        {
            this.Endpoint.Name = endpointConfiguration.ToString();
            ConfigureEndpoint(this.Endpoint, this.ClientCredentials);
        }
        
        public WeatherSoapClient(EndpointConfiguration endpointConfiguration, System.ServiceModel.EndpointAddress remoteAddress) : 
                base(WeatherSoapClient.GetBindingForEndpoint(endpointConfiguration), remoteAddress)
        {
            this.Endpoint.Name = endpointConfiguration.ToString();
            ConfigureEndpoint(this.Endpoint, this.ClientCredentials);
        }
        
        public WeatherSoapClient(System.ServiceModel.Channels.Binding binding, System.ServiceModel.EndpointAddress remoteAddress) : 
                base(binding, remoteAddress)
        {
        }
        
        [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Advanced)]
        System.Threading.Tasks.Task<ServiceReference.GetWeatherResponse> ServiceReference.WeatherSoap.GetWeatherAsync(ServiceReference.GetWeatherRequest request)
        {
            return base.Channel.GetWeatherAsync(request);
        }
        
        public System.Threading.Tasks.Task<ServiceReference.GetWeatherResponse> GetWeatherAsync(string city)
        {
            ServiceReference.GetWeatherRequest inValue = new ServiceReference.GetWeatherRequest();
            inValue.Body = new ServiceReference.GetWeatherRequestBody();
            inValue.Body.city = city;
            return ((ServiceReference.WeatherSoap)(this)).GetWeatherAsync(inValue);
        }
        
        public virtual System.Threading.Tasks.Task OpenAsync()
        {
            return System.Threading.Tasks.Task.Factory.FromAsync(((System.ServiceModel.ICommunicationObject)(this)).BeginOpen(null, null), new System.Action<System.IAsyncResult>(((System.ServiceModel.ICommunicationObject)(this)).EndOpen));
        }
        
        private static System.ServiceModel.Channels.Binding GetBindingForEndpoint(EndpointConfiguration endpointConfiguration)
        {
            if ((endpointConfiguration == EndpointConfiguration.WeatherSoap))
            {
                System.ServiceModel.BasicHttpBinding result = new System.ServiceModel.BasicHttpBinding();
                result.MaxBufferSize = int.MaxValue;
                result.ReaderQuotas = System.Xml.XmlDictionaryReaderQuotas.Max;
                result.MaxReceivedMessageSize = int.MaxValue;
                result.AllowCookies = true;
                return result;
            }
            throw new System.InvalidOperationException(string.Format("Could not find endpoint with name \'{0}\'.", endpointConfiguration));
        }
        
        private static System.ServiceModel.EndpointAddress GetEndpointAddress(EndpointConfiguration endpointConfiguration)
        {
            if ((endpointConfiguration == EndpointConfiguration.WeatherSoap))
            {
                return new System.ServiceModel.EndpointAddress("http://localhost:8080/weather");
            }
            throw new System.InvalidOperationException(string.Format("Could not find endpoint with name \'{0}\'.", endpointConfiguration));
        }
        
        private static System.ServiceModel.Channels.Binding GetDefaultBinding()
        {
            return WeatherSoapClient.GetBindingForEndpoint(EndpointConfiguration.WeatherSoap);
        }
        
        private static System.ServiceModel.EndpointAddress GetDefaultEndpointAddress()
        {
            return WeatherSoapClient.GetEndpointAddress(EndpointConfiguration.WeatherSoap);
        }
        
        public enum EndpointConfiguration
        {
            
            WeatherSoap,
        }
    }
}
