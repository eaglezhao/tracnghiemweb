namespace Network
{
    public interface IMessenger
    {
        void SendMessage(string message);
        void SendMessage(byte[] buffer, MessageType type);
        byte[] ReceiveMessage(out MessageType type);
    }

    public enum MessageType { Command, Image }
}
