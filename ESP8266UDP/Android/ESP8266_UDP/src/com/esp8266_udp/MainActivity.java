package com.esp8266_udp;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InterfaceAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import com.esp8266_udp.R;

import android.support.v7.app.ActionBarActivity;
import android.annotation.SuppressLint;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.os.StrictMode;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

@SuppressLint("NewApi") 
public class MainActivity extends ActionBarActivity {
	
	private int hostPort = 3333;
	private Button btnFind;
	private EditText txtKey;
	private Button btnCmd1;
	private Button btnCmd2;
	private Button btnCmd3;
	private TextView txtStatus;
	private MyDatagramReceiver myDatagramReceiver;
	private String statusString;
	private String localNetwork = "192.168.0.255"; // Change this to match your subnet

    @Override
    protected void onCreate(Bundle savedInstanceState) {
		myDatagramReceiver = new MyDatagramReceiver();
		super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
		StrictMode.setThreadPolicy(policy);	
		

		txtKey = (EditText) findViewById(R.id.txtKey);
		
		txtStatus = (TextView) findViewById (R.id.txtStatus);

    	btnFind = (Button) findViewById(R.id.btnFind);
    	// Save the rule to the pi Server
		btnFind.setOnClickListener(new OnClickListener() {  		
			@SuppressLint("NewApi")
			public void onClick(View v) {
				myDatagramReceiver.discoverServerAddr (localNetwork);
			}
		});    	
		
    	btnCmd1 = (Button) findViewById(R.id.btnCmd1);
    	// Save the rule to the pi Server
		btnCmd1.setOnClickListener(new OnClickListener() {  		
			@SuppressLint("NewApi")
			public void onClick(View v) {
				myDatagramReceiver.sendMessage ("cmd1" );
			}
		});    	
		
    	btnCmd2 = (Button) findViewById(R.id.btnCmd2);
    	// Save the rule to the pi Server
		btnCmd2.setOnClickListener(new OnClickListener() {  		
			@SuppressLint("NewApi")
			public void onClick(View v) {
				myDatagramReceiver.sendMessage ("cmd2" );
			}
		});    	
		
    	btnCmd3 = (Button) findViewById(R.id.btnCmd3);
    	// Save the rule to the pi Server
		btnCmd3.setOnClickListener(new OnClickListener() {  		
			@SuppressLint("NewApi")
			public void onClick(View v) {
				myDatagramReceiver.sendMessage ("cmd3" );
			}
		});    	
		
		myDatagramReceiver.start();
    	
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
	private class MyDatagramReceiver extends Thread {

		public String lastMessage = "";
		
		private boolean bKeepRunning = true;
		private String newMessage = "";
		private int msgNumber = 0;
		private int lastMsgNumber = 0;
		private DatagramSocket socket = null;
		public InetAddress serverAddress = null;
		private String myIpAddress;
		private InetAddress myAddress = null;
		private String receivedData;
		private String serverIpAddress = "";

		public void sendMessage(String msg) {
		
			newMessage = msg;
			incrementMessage();
		}

		public void wait(int millis) {
			try {
				Thread.sleep(millis); // Allow background task to run
			} catch (Exception e) {
			}
		}

		public void incrementMessage() {
			msgNumber = msgNumber + 1;
		}

		public DatagramPacket getPacket() {
			DatagramPacket receivePacket = null;
			byte[] lmessage = new byte[100];

			for (int i = 0; i < 32; i++)
				lmessage[i] = 0;
			receivePacket = new DatagramPacket(lmessage, lmessage.length);
			try {
				socket.receive(receivePacket);
			} catch (IOException io) {
				System.out.println("getPacket, IO Exception getting a packet " + io);
				receivePacket = null;
			}
			return receivePacket;
		}

		public boolean getReceivePacket() {
			DatagramPacket receivedPacket = getPacket();
			boolean gotPacket = false;
			if (receivedPacket != null) {
				String info = new String(receivedPacket.getData());
				receivedData = new String(info.substring(0,receivedPacket.getLength()));
				System.out.println("Received " + receivedData + " from server ");
				postMessage("Received: " + receivedData);
				runOnUiThread(updateTxtStatus);
				gotPacket = true;
			}
			return gotPacket;
		}

		@SuppressLint("NewApi")
		public String getHostName(InterfaceAddress ia) {
			String addr = "";
			int index;
			try {
				String a = new String(ia.toString());
				index = a.indexOf("/", 2);
				addr = new String(a.substring(1, index));
				System.out.println("getHostName(InterfaceAddress): " + addr
						+ ".");
			} catch (Exception e) {
				System.out
						.println("Exception in getHostName(InterfaceAddress)");
			}
			return addr;
		}

		@SuppressLint("NewApi")
		public String getHostName(InetAddress ia) {
			String addr = "";
			int index;
			try {
				String a = new String(ia.toString());
				index = a.indexOf("/", 2);
				if (index == -1)
					addr = new String(a.substring(1));
				else
					addr = new String(a.substring(1, index));
				System.out.println("getHostName(InetAddress):" + addr + ".");
			} catch (Exception e) {
				System.out.println("Exception in getHostName(InetAddress)");
			}
			return addr;
		}

		// String deviceIp: Address on local network like 192.168.0.255
		public String discoverServerAddr(String deviceIp) {
			/*
			 * Send 'find' + txtKey and receive the packet response
			 *  return the address associated with the response from the device
			 */
			String newMessage = "find" + txtKey.getText();
			DatagramPacket sendPacket;
			String addr = "";

			try {
				serverAddress = InetAddress.getByName(deviceIp);
				sendPacket = null;
				sendPacket = new DatagramPacket(newMessage.getBytes(),
						newMessage.length(), serverAddress, hostPort);

				try {
					System.out.println("discoverServerAddr, " + myIpAddress + ": Sending RTS");
					socket.send(sendPacket);
					statusString = new String(newMessage + " to " + deviceIp + ":" + hostPort);
					runOnUiThread(updateTxtStatus);
					
					getPacket(); // First, we receive the echo.
					// Now receive the packet from the server
					DatagramPacket serverPacket = getPacket(); 
					
					InetAddress comm_ip = serverPacket.getAddress();
					addr = new String(getHostName(comm_ip));
					System.out.println("Server discovered at: ");
					System.out.println(addr);
					statusString = new String ("Server address discovered at: " + addr);
					runOnUiThread(updateTxtStatus);

				} catch (IOException i) {
					System.out.println("Could not send packet in discover server addr");
				}
			} catch (Exception e) {
				System.out.println("Could not getByName (" + deviceIp + "(");
			}
			return addr;
		}
		
		@SuppressLint("NewApi")
		public String ip() {
			String addr = "";
			try {
				Enumeration<NetworkInterface> nis = NetworkInterface
						.getNetworkInterfaces();
				NetworkInterface ni;
				while (nis.hasMoreElements() && (myAddress == null)) {
					ni = nis.nextElement();
					if (!ni.isLoopback() && ni.isUp()) {
						for (InterfaceAddress ia : ni.getInterfaceAddresses()) {
							// filter for ipv4/ipv6
							if (ia.getAddress().getAddress().length == 4) {
								// 4 for ipv4, 16 for ipv6
								try {
									addr = getHostName(ia);
								} catch (Exception e) {
									System.out.println("Some error");
								}
								System.out.println("my ip address: " + addr
										+ ".");
								statusString = new String ( "my ip address = " + addr);
								break;
							}
						}
					}
				}
			} catch (SocketException s) {
				// NOTE: in AndroidManifest.xml,  
				//       <uses-permission android:name="android.permission.INTERNET" />
				System.out.println("Exception in ip()");
			}
			return addr;
		}
		
		public void run() {
			System.out.println ( "DataGramReceiver.run()");
			
			if (serverIpAddress.equals("")) {
				try {
					System.out.println("Get the socket.");
					socket = new DatagramSocket(hostPort);
					System.out.println("setSoTimeout");
					socket.setSoTimeout(1000); // Allow enough time for server to respond
					System.out.println("setReuseAddress");
					socket.setReuseAddress(true); // why?
				} catch (Exception e) {
					Log.e("UDP", "C: Error", e);
				}

				myIpAddress = new String(ip());
	
				while (serverIpAddress.equals ("")) {
					// Get the String for server Address
					serverIpAddress = new String(discoverServerAddr(localNetwork)); // Get server address
					wait (500);
				}
				System.out.println("serverIpAddress: " + serverIpAddress);
				// Get the InetAddress for serverAddress
				try {
					serverAddress = InetAddress.getByName(serverIpAddress);
				} catch (Exception e) {
					Log.e("UDP", "C: Error", e);
				}
			
				//readPoolInfo();
				statusString = new String ("Server: " + serverIpAddress);
				runOnUiThread(updateTxtStatus);
			}
			
			// Get the rules so you can show the first one
			sendMessage("Show");
			
			try {
				DatagramPacket sendPacket;
				System.out.println ( "Run forever");
				while (bKeepRunning) {
					
				
					try {
						if (msgNumber == lastMsgNumber) {
							wait(10);
						} else { 
							// if (msgNumber != lastMsgNumber)
							//inProgress = true;
							lastMsgNumber = msgNumber;
							sendPacket = new DatagramPacket(
									newMessage.getBytes(), newMessage.length(),
									serverAddress, hostPort);
							System.out.println("Send:");
							System.out.println(newMessage);
							System.out.println("to ");
							System.out.println(serverIpAddress); 
							socket.send(sendPacket);
							
							postMessage("Sending: " + newMessage);
							if (getReceivePacket()) {
								postMessage(receivedData);
								if (receivedData.equals("ACK")) {
									postMessage("Server Acknowledged");
								} else {
							  	   postMessage ("Server failed to ACK");
								}
							}
							else {
							   postMessage ("Missing response msg");
							}
						}
					} catch (Throwable e) {
						e.printStackTrace();
						Log.e("UDP", "D: Error", e);
					}
					
				}

			} catch (Exception e) {
				Log.e("UDP", "C: Error", e);
			}
			
			
		}
		
		public void postMessage(String msg) {
			lastMessage = msg;
			runOnUiThread(updateTxtStatus);
		}
		
	}
	
	private Runnable updateTxtStatus = new Runnable() {
		/* Update the text field */
		public void run() {
			txtStatus.setText(statusString);
		}
	};
	
		
    
}
