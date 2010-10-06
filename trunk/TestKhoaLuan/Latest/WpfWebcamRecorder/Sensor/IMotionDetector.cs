using System.Drawing;

namespace Sensor
{	
	public interface ISensor
	{
        bool IsActive { get; set; }
        bool IsAlarming { get; }
        double Sensitivity { get; set; }
		void ProcessFrame( ref Bitmap image );
		void Reset( );
	}
}
