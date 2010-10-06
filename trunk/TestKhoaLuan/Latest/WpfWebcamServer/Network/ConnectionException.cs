using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Network
{
    class ConnectionException : ApplicationException
    {
        public ConnectionException() : base("A ConnectionException has occured in your program") { }

        public ConnectionException(string message) : base(message) { }

        public ConnectionException(string message, Exception innerException) : base(message, innerException) { }
    }
}
