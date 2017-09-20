#include <string>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <stdint.h>
#include <tinyb.hpp>
#include <iomanip>

#include <vector>
#include <thread>
#include <atomic>
#include <csignal>

#include <fstream>
#include <iostream>

using namespace tinyb;
using namespace std;

std::atomic<bool> running(true);

void signal_handler(int signum)
{
    if (signum == SIGINT) {
        running = false;
    }
}

vector<string> list_of_tiny;

bool check_address(string s) {
	bool output = false;
	
	for (int i = 0; i < list_of_tiny.size(); i++) {
		if (s == list_of_tiny[i]) {
			return true;
		}
	} 
	
	return output;
}

vector<int> s_data;
vector<int> s_prev;
vector<int> e_data;
vector<int> e_prev;

void split_data(unsigned char *data) {
	int tempx = 0;
	int tempy = 0; 
	int tempz = 0;
	if (data[0] == 's') {
		s_data.clear();
		for (int i = 2; i < 7; i++) {
			tempx = tempx * 10;
			tempx += data[i] - '0';
		}
		for (int i = 8; i < 13; i++) {
			tempy = tempy * 10;
			tempy += data[i] - '0';			
		}
		for (int i = 14; i < 19; i++) {
			tempz = tempz * 10;
			tempz += data[i] - '0';			
		}
		if (data[1] == '-') {
			tempx = -1 * tempx;
		}
		if (data[7] == '-') {
			tempy = -1 * tempy;
		}
		if (data[13] == '-') {
			tempz = -1 * tempz;
		}
		
		//cout << tempx << ", " << tempy << ", " << tempz;// << endl;
		s_data.push_back(tempx);
		s_data.push_back(tempy);
		s_data.push_back(tempz);
		return;
	}
	else if (data[0] == 'e') {
		e_data.clear();
		for (int i = 2; i < 7; i++) {
			tempx = tempx * 10;
			tempx += data[i] - '0';
		}
		for (int i = 8; i < 13; i++) {
			tempy = tempy * 10;
			tempy += data[i] - '0';			
		}
		for (int i = 14; i < 19; i++) {
			tempz = tempz * 10;
			tempz += data[i] - '0';			
		}
		if (data[1] == '-') {
			tempx = -1 * tempx;
		}
		if (data[7] == '-') {
			tempy = -1 * tempy;
		}
		if (data[13] == '-') {
			tempz = -1 * tempz;
		}
		//cout << tempx << ", " << tempy << ", " << tempz;// << endl;
		e_data.push_back(tempx);
		e_data.push_back(tempy);
		e_data.push_back(tempz);		
		return;
	}
	else if (data[0] == '[') {
		return;
	}
	else {
		cout << "wrong data structure" << endl;
		return;
	}
}

void ReadCin(atomic<bool>& run) {
	string buffer;
	
	while (run.load()) {
		cin >> buffer;
		
		if (buffer == "e") {
			run.store(false);
		}
	}
}

int main(int argc, char **argv) 
{

	BluetoothManager *manager = nullptr;
	try {
		manager = BluetoothManager::get_bluetooth_manager();
	} catch(const std::runtime_error& e) {
		cerr << "Error while initializing libtinyb: " << e.what() << endl;
		exit(1);
	}

    cout << "Initialization was succesful." << endl;
	
	/* Start the discovery of devices */
    bool ret = manager->start_discovery();
    if (!ret) {
		cout << "Failed to start Discovery" << endl;
		return 0;
	}
    cout << "Discovering BLE devices" << endl;
    
	std::unique_ptr<BluetoothAdapter> adapter = manager->get_default_adapter();
	//BluetoothDevice *ess_device = NULL;
	BluetoothDevice *sensor_tag = NULL;
	//BluetoothGattService *tinyb_service = NULL;
	
	list_of_tiny.push_back("E2:48:FF:ED:4E:3D"); 
	list_of_tiny.push_back("F2:60:3B:58:BA:EC");
	
	vector<unique_ptr<BluetoothDevice>> list_devices;
	//for (int i = 0; i < 15; i++) {
	while(1) {
//		cout << "Discovered Devices: " << endl;
		list_devices = manager->get_devices();
		
		for (auto it = list_devices.begin(); it != list_devices.end(); it++) {
			if ((*it)->get_rssi() != 0) {
				if (check_address((*it)->get_address())) {
					cout << "RSSI = " << (*it)->get_rssi() << " ";
//					cout << "Class = " << (*it)->get_class_name() << " ";
					cout << "Path = " << (*it)->get_object_path() << " ";
//					cout << "Name = " << (*it)->get_name() << " ";
					//cout << "Connected = " << (*it)->get_connected() << " ";
					cout << endl;
				}



				if ((*it)->get_address() == "E2:48:FF:ED:4E:3D") {
					cout << "E2:48:FF:ED:4E:3D address found" << endl;
					sensor_tag = (*it).release();
//					sensor_tag->connect();
				}
				else if ((*it)->get_address() == "F2:60:3B:58:BA:EC") {
					cout << "address found" << endl;
					sensor_tag = (*it).release();
					//sensor_tag->connect();
				}
			}
		}
		
		if (sensor_tag != nullptr) {
			break;
		}
		else {
			std::this_thread::sleep_for(std::chrono::seconds(4));
			//cout << endl;
		}
	}

	if (sensor_tag == nullptr) {
		cout << "Could not find device " << "E2:48:FF:ED:4E:3D" << endl;
		return 0; 
	}
	
	sensor_tag->connect();

	/* Wait for the device to come online */
	cout << "Device Coming Online... " << endl;
	cout << "Obtaining Sensor Tag services" << endl; 
		
	std::this_thread::sleep_for(std::chrono::seconds(4));
	auto service_list = sensor_tag->get_services();
	if (!service_list.empty()) {
		cout << "Service List size: " << service_list.size() + 1<< endl;
	}
	else {
		cout << "Service List is empty" << endl;
	}
	auto uuid_list = sensor_tag->get_uuids();
	if (!uuid_list.empty()) {
		cout << "UUID LIST: " << endl;
		for (int i = 0; i < uuid_list.size(); i++) {
			cout << uuid_list[i] << endl;
		}
		cout << "END OF LIST" << endl;
	}
	else {
		cout << "UUID_LIST is empty" << endl;
	}
	/*
	auto data_list = sensor_tag->get_service_data();
	if (!data_list.empty()) {
		cout << "I HAVE DATA" << endl;
	}
	else {
		cout << "NO DATA" << endl;
	}
	*/
	std::string service_uuid("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
	cout << "Waiting for service " << service_uuid << " to be discovered" << endl;
	auto tinyb_service = sensor_tag->find(&service_uuid);
/*
	cout << "Stopping Discovery... " << endl;
	ret = manager->stop_discovery();
	cout << "Discovery Stopped" << endl;	
*/
    cout << "getting tx_uuid" << endl;
    auto tx_uuid = string("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
    auto tx_service = tinyb_service->find(&tx_uuid);
    
    
    int num = 1;
    
    ofstream output_file("LOG/output" + to_string(num) + ".txt"); 
    
	if (!output_file.is_open()) {
		cout << "Error opening output file" << endl;
		return 0;
	}
    
    
    cout << "while loop here lmao" << endl;
    /*
    bool exit = false;
    char key = ' ';
    */
    
    bool init = true;
    int sec = 0;
    atomic<bool> run(true);
    thread cinThread(ReadCin, ref(run));
    
    //system("exec rm -r LOG/*");
      
    while(run.load()) {
		/*
		cin >> key;
		if (key == 'e') {
			exit = true;
			cout << "EXIT" << endl;
			return 0;
		}
		*/
		try {
			/*
			if (num == 5) {
				
			}
			
			ofstream output_file("LOG/output" + to_string(num) + ".txt"); 
			num++;
			if (!output_file.is_open()) {
				cout << "Error opening output file" << endl;
				return 0;
			}
			*/
			std::vector<unsigned char> response = tx_service->read_value();
			
			unsigned char *data;
			unsigned int size = response.size();
			if (size <= 0) {
				cout << "NO DATA COMING IN" << endl;
			}
			else {
				
				data = response.data();
				
				//cout << "raw data = "; //<< data << endl;
				
				/*
				for (unsigned i = 0; i < size; i++) {
					cout << data[i];
					//cout << std::hex << static_cast<int>(data[i]);
				}
				*/			
				split_data(data);
				if (!init) {
					if (sec < 10 && (s_prev == s_data || e_prev == e_data)) {
						sec++;
						//cout << sec << endl;
					}
					else {
						sec = 0;
						s_prev = s_data;
						e_prev = e_data;
						cout << "converted data = ";
						cout << s_data[0] << " " << s_data[1] << " " << s_data[2] << " ";
						cout << e_data[0] << " " << e_data[1] << " " << e_data[2] << endl;
						
						output_file  << s_data[0]/10000.0 << " " << s_data[1]/10000.0 << " " << s_data[2]/10000.0 << " "  << e_data[0]/10000.0 << " " << e_data[1]/10000.0 << " " << e_data[2]/10000.0 << endl;
					}
				}
				else {					
					if (!s_data.empty() && !e_data.empty()) {
						init = false;
					}
				}
				/*
				for (int i = 0; i < 3; i++) {
					if (data[0] == 's') {
						if (i == 0) {
							cout << "s";
							output_file << "s";
						}
						cout << setprecision(2) << fixed << s_data[i]/10000.0;
						output_file << s_data[i];// << ',';
					}
					else if (data[0] == 'e') {
						if (i == 0) {
							cout << "e";
							output_file << "e";
						}
						cout << setprecision(2) << fixed << e_data[i]/10000.0;
						output_file << e_data[i];// << ',';
					}
					
					if (i != 2) {
						output_file << ',';
						cout << ", ";
					}
				}
				*/
				//cout << endl;
				
				//output_file.close();
			
			
				//output_file << "\n";
			}
		}
		catch (exception &e) {
			cout << "ERROR: " << e.what() << endl;
			//output_file.close();
			sensor_tag->disconnect();
			//break;
		}
	}
	output_file.close();
    cout << "OH WOW" << endl;
	run.store(false);
	cinThread.join();
	
	return 0;
}
//sudo ./src/catkin/bin/catkin_make_isolated --install -DCMAKE_BUILD_TYPE=Release --install-space /opt/ros/indigo -j2
