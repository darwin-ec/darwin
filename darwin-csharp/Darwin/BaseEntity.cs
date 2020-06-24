using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin
{
    public class BaseEntity
    {
        [JsonIgnore]
        public long ID { get; set; }
    }
}
