/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.hellojni;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.ImageView.ScaleType;
import android.os.AsyncTask;
import android.os.Bundle;
import java.io.*;
import java.net.*;
import java.nio.Buffer;
import java.nio.channels.Channels;
import java.util.TooManyListenersException;


public class HelloJni extends Activity
{
	public final int COM_SOCKET = 4115;
	public final int DATA_SOCKET = 4116;
    /** Called when the activity is first created. */
	
	ImageView image;
	DownloadFilesTask as;
	
	Socket comSocket;
	Socket dataSocket;
	static public HelloJni mThis;
    @Override
    /* (non-Javadoc)
     * @see android.app.Activity#onCreate(android.os.Bundle)
     */
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        /* Create a TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        mThis = this;
        image = new ImageView(this);
        as = new DownloadFilesTask();
        
        
        setContentView(image);
        //setContentView(R.layout.main);

        
       
        
        comSocket = null;
        dataSocket = null;
		try {
			
			//clientSocket = new Socket("127.0.0.1", 5000);
			InetAddress serverAddr = InetAddress.getByName("10.218.9.212");//10.66.3.44 is my pc' IP 
			//Log.d("TCP", "C: Connecting..."); 
			comSocket = new Socket(serverAddr, COM_SOCKET);
			dataSocket = new Socket(serverAddr, DATA_SOCKET);
			as.execute(comSocket, dataSocket);
			//ServerSocket svSocket = new ServerSocket(4500);
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
		
        //BufferedReader inFromUser =
        //               new BufferedReader(new InputStreamReader(System.in));
        
//		 String FromServer = null;
//	     String ToServer = null;
//        
//           
//        PrintWriter outToServer = null;
//        BufferedReader inFromServer = null;
//		try {
//			outToServer = new PrintWriter(
//			   comSocket.getOutputStream(),true);
//			inFromServer = new BufferedReader(new InputStreamReader(
//			           comSocket.getInputStream()));
//		} catch (IOException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//           
//        
//           
//		while (true)
//		{
//			try {
//				//Thread.sleep(1000);
//				FromServer = inFromServer.readLine();
//
//				if ( FromServer.equals("q") || FromServer.equals("Q"))
//				{
//					comSocket.close();
//					break;
//				}
//				else
//
//				{
//					cout("RECIEVED:" + FromServer);
//					cout("SEND(Type Q or q to Quit):");
//
//					//ToServer = inFromUser.readLine();
//					ToServer = "Welcome from client";
//
//					if (ToServer.equals("Q") || ToServer.equals("q"))
//					{
//						outToServer.println (ToServer) ;
//						comSocket.close();
//						break;
//					}
//
//					else
//					{
//						outToServer.println(ToServer);
//					}
//				}
//			} catch (IOException e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//			}
//
//
//
//
//		}   
        
//        String fromclient;
//        String toclient;
//         
//        ServerSocket Server = null;
//		try {
//			Server = new ServerSocket (4500);
//		} catch (IOException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//        
//        System.out.println ("TCPServer Waiting for client on port 4500");
//
//		while (true) {
//			Socket connected;
//			try {
//				connected = Server.accept();
//				cout(" THE CLIENT" + " "
//						+ connected.getInetAddress() + ":"
//						+ connected.getPort() + " IS CONNECTED ");
//
//				//BufferedReader inFromUser = new BufferedReader(
//				//		new InputStreamReader(System.in));
//
//				BufferedReader inFromClient = new BufferedReader(
//						new InputStreamReader(connected.getInputStream()));
//
//				PrintWriter outToClient = new PrintWriter(connected
//						.getOutputStream(), true);
//
//				while (true) {
//
//					System.out.println("Server: SEND(Type Q or q to Quit):");
//					//toclient = inFromUser.readLine();
//					toclient =  "Welcome to server";
//
//					if (toclient.equals("q") || toclient.equals("Q")) {
//						outToClient.println(toclient);
//						connected.close();
//						break;
//					} else {
//						outToClient.println(toclient);
//					}
//
//					fromclient = inFromClient.readLine();
//
//					if (fromclient.equals("q") || fromclient.equals("Q")) {
//						connected.close();
//						break;
//					}
//
//					else {
//						System.out.println("Server: RECIEVED:" + fromclient);
//					}
//
//				}
//			} catch (IOException e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//			}
//
//		}
    }

    /* A native method that is implemented by the
     * 'hello-jni' native library, which is packaged
     * with this application.
     */
    public void cout(String s)
    {
    	//tv.setText(tv.getText() + "\n" + s);
    }
    
    public static void update(byte... progress)
    {
    	BitmapFactory b = new BitmapFactory();
        final Bitmap bm = b.decodeByteArray(progress, 0, 2572);
        
        
        HelloJni.mThis.runOnUiThread(new Runnable()
	      {
	         public void run()
	         {
	        	 ImageView img = HelloJni.mThis.image;//(ImageView)HelloJni.mThis.findViewById(R.id.picview);
	        	 HelloJni.mThis.image.setScaleType(ScaleType.CENTER);
	        	 img.setImageBitmap(bm);
	        	 img.invalidate();
	        	 //HelloJni.mThis.image.setScaleType(ScaleType.FIT_CENTER);
	        	 //HelloJni.mThis.image.setAdjustViewBounds(true);
	        	 //HelloJni.mThis.image.setMaxWidth(bm.getWidth());
	        	 //HelloJni.mThis.image.setMaxHeight(bm.getHeight());
	         }
	      });
        
    }
    public native String  stringFromJNI();

    /* This is another native method declaration that is *not*
     * implemented by 'hello-jni'. This is simply to show that
     * you can declare as many native methods in your Java code
     * as you want, their implementation is searched in the
     * currently loaded native libraries only the first time
     * you call them.
     *
     * Trying to call this function will result in a
     * java.lang.UnsatisfiedLinkError exception !
     */
    public native String  unimplementedStringFromJNI();

    /* this is used to load the 'hello-jni' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.HelloJni/lib/libhello-jni.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("hello-jni");
    }
}

// The contents of this file are subject to the Mozilla Public
// License Version 1.1 (the "License"); you may not use this file
// except in compliance with the License. You may obtain a copy of
// the License at http://www.mozilla.org/MPL/
// 
// Software distributed under the License is distributed on an "AS
// IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
// implied. See the License for the specific language governing
// rights and limitations under the License.
// 
// The Original Code is eBus.
// 
// The Initial Developer of the Original Code is Charles W. Rapp.
// Portions created by Charles W. Rapp are
// Copyright (C) 2001 Charles W. Rapp.
// All Rights Reserved.
// 
// Contributor(s): 
//
// RCS ID
// $Id: AsyncSocket.java,v 1.0 2003/11/20 02:02:06 charlesr Exp $
//
// Change Log
// $Log: AsyncSocket.java,v $
// Revision 1.0  2003/11/20 02:02:06  charlesr
// Initial revision
//

class DownloadFilesTask extends AsyncTask<Socket, Byte, Integer> {
	private int bmLength;
	public static final int ERROR = 0;
	public static final int SUCCESS = 1;
	public static final int STATE_BEGIN = 1;
	public static final int STATE_TRANSFER = 2;
	public static final int STATE_END = 3;
	public static final int SERVER_CMD = 0;
	public static final int SERVER_PAR = 1;
	int curState;
	Socket comSocket;
	Socket dataSocket;
	
    protected Integer doInBackground(Socket... socket) {
//        int count = urls.length;
//        long totalSize = 0;
//        for (int i = 0; i < count; i++) {
//            totalSize += Downloader.downloadFile(urls[i]);
//            publishProgress((int) ((i / (float) count) * 100));
//        }
//        
//        return totalSize;
    	
        String FromServer = null;
	    String ToServer = null;
        
        PrintWriter outToServer = null;
        BufferedReader inFromServer = null;
        
        comSocket = socket[0];
        dataSocket = socket[1];
		try {
			outToServer = new PrintWriter(new OutputStreamWriter(
			   comSocket.getOutputStream()),true);
			inFromServer = new BufferedReader(new InputStreamReader(
			           comSocket.getInputStream()));
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		outToServer.println("REQUEST");
		outToServer.flush();
		curState = STATE_BEGIN;
           
		while (true)
		{
//			try {
//				//Thread.sleep(1000);
//				FromServer = inFromServer.readLine();
//
//				if ( FromServer.equals("q") || FromServer.equals("Q"))
//				{
//					comSocket.close();
//					break;
//				}
//				else
//
//				{
//					//cout("RECIEVED:" + FromServer);
//					//cout("SEND(Type Q or q to Quit):");
//
//					//ToServer = inFromUser.readLine();
//					ToServer = "Welcome from client";
//
//					if (ToServer.equals("Q") || ToServer.equals("q"))
//					{
//						outToServer.println (ToServer) ;
//						comSocket.close();
//						break;
//					}
//
//					else
//					{
//						outToServer.println(ToServer);
//					}
//				}
//			} catch (IOException e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//			}
			try {
				FromServer = inFromServer.readLine();
				switch(curState)
				{
					case STATE_BEGIN:
						String[] tokens = FromServer.split(" ");
						if(tokens[SERVER_CMD].equals("ACCEPT"))
						{
							outToServer.println("LENGTH");
							outToServer.flush();
						}if(tokens[SERVER_CMD].equals("LENGTH"))
						{
							if(tokens.length < 2)
							{
								outToServer.println("ERROR");
								outToServer.flush();
								return ERROR;
							
							}else
							{	
								outToServer.println("BEGIN");
								outToServer.flush();
								bmLength = Integer.parseInt(tokens[SERVER_PAR]);
								curState = STATE_TRANSFER;
							}
						}
						break;
					case STATE_TRANSFER:
						InputStream stream = dataSocket.getInputStream();
						//bOut.writeTo(dataSocket.getInputStream());
						//InputStream stream = Channels.newInputStream(aSocketChannel);
//						 InputStream stream = aSocketChannel.socket().getInputStream ();	 // either way
						int length = 0;
						byte[] bytes = new byte[bmLength];

							while (length < bmLength) {
								int count = stream.read(bytes, length, bmLength - length);
								
								if (count < 0) {
									byte[] buffer = new byte[length];
									
									System.arraycopy(bytes, 0, buffer, 0, length);
									
									//return buffer;
									onProgressUpdate(buffer);
									return ERROR;
								}
								
								length += count;
							}
							
							//return bytes;
							onProgressUpdate(bytes);
							curState = STATE_END;
						break;
					case STATE_END:
						return SUCCESS;
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

		}  
    	//return SUCCESS;
    }

    protected void onProgressUpdate(byte... progress) {
    	
        //b.copyPixelsFromBuffer(src);
        
        
    	HelloJni.update(progress);
        
        //setProgressPercent(progress[0]);
    }

    protected void onPostExecute(int result) {
        //showDialog("Downloaded " + result + " bytes");
    }
}



