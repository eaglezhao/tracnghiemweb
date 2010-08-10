using System;
using System.Drawing;

namespace Imaging
{
    public static class Imaging
    {
        public static void AddText(this Bitmap image, Font fontOverlay, float top, float left, string text)
        {
            // Prepare to put the specified string on the image
            Bitmap bitmapOverlay = new Bitmap(image.Width, image.Height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            Graphics g = Graphics.FromImage(bitmapOverlay);
            g.Clear(System.Drawing.Color.Transparent);
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            SizeF d = g.MeasureString(text, fontOverlay);

            // Now draw the string on the image
            g.FillRectangle(System.Drawing.Brushes.Black, left - 3, top - 3, d.Width + 6, d.Height + 6);
            g.DrawString(text, fontOverlay, System.Drawing.Brushes.White,
                left, top, System.Drawing.StringFormat.GenericTypographic);
            g.Dispose();

            g = Graphics.FromImage(image);
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

            // draw the overlay bitmap over the video's bitmap
            g.DrawImage(bitmapOverlay, 0, 0, bitmapOverlay.Width, bitmapOverlay.Height);

            // dispose of the various objects
            g.Dispose();
        }
    }
}
