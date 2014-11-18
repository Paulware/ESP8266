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
import android.content.pm.ActivityInfo;
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
	
	private String localNetwork = ""; // Change this to match your subnet
	private int hostPort = 3333;
	private Button btnFind;
	private EditText txtKey;
	private Button btnCmd1;
	private Button btnCmd2;
	private Button btnCmd3;
	private TextView txtStatus;
	private MyDatagramReceiver myDatagramReceiver;
	private String statusString;
	private DatagramPacket sendPacket;
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		myDatagramReceiver.kill();
		super.onDestroy();
	}
	

    @Override
    protected void onCreate(Bundle savedInstanceState) {
		myDatagramReceiver = new MyDatagramReceiver();
		super.onCreate(savedInstanceState);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.activity_main);

        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
		StrictMode.setThreadPolicy(policy);	
		
		txtKey = (EditText) findViewById(R.id.txtKey);
		
		txtStatus = (TextView) findViewById (R.id.txtStatus);

		// Find the device with the given key
    	btnFind = (Button) findViewById(R.id.btnFind);
 		btnFind.setOnClickListener(new OnClickListener() {  		
			@SuppressLint("NewApi")
			public void onClick(View v) {
				myDatagramReceiver.discoverDeviceAddr (localNetwork);
			}
		});    	
		
    	btnCmd1 = (Button) findViewById(R.id.btnCmd1);
		btnCmd1.setOnClickListener(new OnClickListener() {  		
			@SuppressLint("NewApi")
			public void onClick(View v) {
				myDatagramReceiver.sendMessage ("cmd1" );
			}
		});    	
		
    	btnCmd2 = (Button) findViewById(R.id.btnCmd2);
		btnCmd2.setOnClickListener(new OnClickListener() {  		
			@SuppressLint("NewApi")
			public void onClick(View v) {
				myDatagramReceiver.sendMessage ("cmd2" );
			}
		});    	
		
    	btnCmd3 = (Button) findViewById(R.id.btnCmd3);
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
		
		private boolean bKeepRunning = true;
		private String newMessage = "";
		private int msgNumber = 0;
		private int lastMsgNumber = 0;
		private DatagramSocket socket = null;
		public InetAddress deviceAddress = null;
		private String myIpAddress;
		private InetAddress myAddress = null;
		private String receivedData;
		private String deviceIpAddress = "";

		public void kill() {
		    socket.close();
		}
		

		public void sendMessage(String msg) {
		
			newMessage = msg;
			msgNumber = msgNumber + 1;
		}

		public void wait(int millis) {
			try {
				Thread.sleep(millis); // Allow background task to run
			} catch (Exception e) {
			}
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
				System.out.println("Received " + receivedData + " from device");
				statusString = "Received: " + receivedData;
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
		public String discoverDeviceAddr(String deviceIp) {
			/*
			 * Send 'find' + txtKey and receive the packet response
			 *  return the address associated with the response from the device
			 */
			String newMessage = "findkey"; //  + txtKey.getText();
			String addr = "";
			int len;

			try {
				deviceAddress = InetAddress.getByName(deviceIp);
				System.out.println ( "deviceAddress: " + deviceAddress + " hostPort: " + hostPort);
				
		        len = newMessage.length();
		        System.out.println ( "newMessage.length(): " + len );
		        System.out.println ( "newMessage:" + newMessage);
				sendPacket = new DatagramPacket(newMessage.getBytes(),newMessage.length(), deviceAddress, hostPort);

				try {
					System.out.println("discoverDeviceAddr, " + myIpAddress + ": Sending RTS");
					socket.send(sendPacket);
					try {
						statusString = new String(newMessage + " to " + deviceIp + ":" + hostPort);
						runOnUiThread(updateTxtStatus);
					
						getPacket(); // First, we receive the echo.
						// Now receive the packet from the device
						DatagramPacket devicePacket = getPacket(); 
						if (devicePacket == null) 
							System.out.println ( "Got a null response (getPacket)");
						else {
							try {			
								InetAddress comm_ip = devicePacket.getAddress();
								addr = new String(getHostName(comm_ip));
								System.out.println("Device discovered at: ");
								System.out.println(addr);
								statusString = new String ("Device address discovered at: " + addr);
								runOnUiThread(updateTxtStatus);
							} catch (Exception k) {
								System.out.println ( "Could not getAddress " + k );
							}
						} 
					} catch (Exception j) {
					    System.out.println ( "Could not get packet" + j);
					}

				} catch (Exception i) {
					System.out.println("Could not socket.send: " + i);
				}
			
			} catch (Exception e) {
				System.out.println("Could not create a sendPacket1: " + e);
			}
			return addr;
		}
		
		@SuppressLint("NewApi")
		// Get my ip address
		public String ip() {
			String addr = "";
			int lastDot = 0;
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
								// update localNetwork
                                lastDot = addr.lastIndexOf ( ".");                                
                                localNetwork = new String (addr.substring (0,lastDot) + ".255"); // ".70"); // ".255");
                                System.out.println ( "localNetwork:" + localNetwork);
                                
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
			
			if (deviceIpAddress.equals("")) {
				try {
					System.out.println("Get the socket.");
					socket = new DatagramSocket(hostPort);
					System.out.println("setSoTimeout");
					socket.setSoTimeout(3000); // Allow enough time for the device to respond
					System.out.println("setReuseAddress");
					socket.setReuseAddress(true); // why?
				} catch (Exception e) {
					Log.e("UDP", "C: Error", e);
				}

				myIpAddress = new String(ip());
	
				while (deviceIpAddress.equals ("")) {
					// Get the String for device Address
					deviceIpAddress = new String(discoverDeviceAddr(localNetwork)); // Get device address
					wait (2500);
				}
				System.out.println("deviceIpAddress: " + deviceIpAddress);
				// Get the InetAddress for deviceAddress
				try {
					deviceAddress = InetAddress.getByName(deviceIpAddress);
				} catch (Exception e) {
					Log.e("UDP", "C: Error", e);
				}
			
				statusString = new String ("Device: " + deviceIpAddress);
				runOnUiThread(updateTxtStatus);
			}
			
			try {
				// DatagramPacket sendPacket;
				System.out.println ( "Run forever");
				while (bKeepRunning) {
									
					try {
						if (msgNumber != lastMsgNumber) {
							lastMsgNumber = msgNumber;
							sendPacket = new DatagramPacket(
									newMessage.getBytes(), newMessage.length(),
									deviceAddress, hostPort);
							System.out.println("Send:" + newMessage + " to " + deviceIpAddress);
							socket.send(sendPacket);							
							/*
							while (true) {
								socket.send(sendPacket);							
								//statusString = "Sending: " + newMessage;
								//runOnUiThread (updateTxtStatus);
								if (getReceivePacket()) {
									//statusString = receivedData;
									//runOnUiThread (updateTxtStatus);
									System.out.println ( "Got back response: " + receivedData + ".");
									if (receivedData.trim().equals("ACK")) {
										statusString = "Device Ack'd " + newMessage;
										runOnUiThread (updateTxtStatus);
										break;
									} else {
										statusString = "Device failed to ACK";
										runOnUiThread (updateTxtStatus);	
									}
								}	
							}
							*/
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
				
	}
	
	private Runnable updateTxtStatus = new Runnable() {
		/* Update the text field */
		public void run() {
			txtStatus.setText(statusString);
		}
	};
	
		
    
}
