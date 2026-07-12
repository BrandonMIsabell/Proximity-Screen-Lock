package com.example.screenlockapp

// --- CORE APP IMPORTS ---
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import com.example.screenlockapp.ui.navigation.appNavScreen
import com.example.screenlockapp.theme.ScreenLockappTheme

// --- BLUETOOTH & SCANNING IMPORTS ---
import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.util.Log
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.setValue
import androidx.core.app.ActivityCompat

class MainActivity : ComponentActivity() {

    // CENTRALIZED APP STATE: Redraws the UI whenever its value changes
    private var connectionStatus by mutableStateOf("Disconnected")

    // 1. BLUETOOTH HARDWARE SETUP
    private val bluetoothManager by lazy {
        getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
    }
    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        bluetoothManager.adapter
    }

    private val bleScanner: BluetoothLeScanner? get() = bluetoothAdapter?.bluetoothLeScanner
    private var isScanning = false

    // Active GATT Profile Holder
    private var bluetoothGatt: BluetoothGatt? = null

    // 2. BLUETOOTH ENABLE LAUNCHER
    private val enableBluetoothLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { /* Not needed */ }

    // 3. PERMISSION REQUEST LAUNCHER
    private val permissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        val canConnect = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            permissions[Manifest.permission.BLUETOOTH_CONNECT] == true
        } else true

        if (canConnect && bluetoothAdapter?.isEnabled == false) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            enableBluetoothLauncher.launch(enableBtIntent)
        }
    }

    // 4. THE HANDSHAKE CALLBACK (Listens for actual connection completion)
    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
            super.onConnectionStateChange(gatt, status, newState)

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d("BLE_TEST", "Gatt connection established with ESP32!")
                connectionStatus = "Connected to ESP32"
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d("BLE_TEST", "Gatt disconnected from ESP32.")
                connectionStatus = "Disconnected"
                bluetoothGatt?.close()
                bluetoothGatt = null
            }
        }
    }

    // 5. THE BLE SCAN CALLBACK (Finds the advertisement frame)
    private val bleScanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            super.onScanResult(callbackType, result)

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S &&
                ActivityCompat.checkSelfPermission(this@MainActivity, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                return
            }

            val deviceName = result.device.name

            if (deviceName == "Proximity_ESP32") {
                Log.d("BLE_TEST", "FOUND TARGET! Stopping scan and initiating GATT connection...")

                // Stop hunting immediately
                stopBleScanSilently()

                // Establish direct physical connection
                connectionStatus = "Connecting..."
                bluetoothGatt = result.device.connectGatt(this@MainActivity, false, gattCallback)
            }
        }

        override fun onScanFailed(errorCode: Int) {
            super.onScanFailed(errorCode)
            connectionStatus = "Scan Failed: Error $errorCode"
            isScanning = false
        }
    }

    // CENTRALIZED TRIGGER LOGIC
    private fun startEspBluetoothSearch() {
        if (bluetoothAdapter == null) {
            connectionStatus = "Device Doesn't Support Bluetooth"
            return
        }

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
            connectionStatus = "Missing Scan Permissions"
            return
        }

        // If we are already connected or connecting, clicking the button acts as a disconnect trigger
        if (bluetoothGatt != null) {
            disconnectGatt()
            return
        }

        if (!isScanning) {
            connectionStatus = "Searching for ESP32..."
            isScanning = true
            bleScanner?.startScan(bleScanCallback)
        } else {
            stopBleScan()
        }
    }

    private fun stopBleScan() {
        stopBleScanSilently()
        connectionStatus = "Disconnected"
    }

    private fun stopBleScanSilently() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED) {
            bleScanner?.stopScan(bleScanCallback)
        }
        isScanning = false
    }

    private fun disconnectGatt() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S &&
            ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            return
        }
        connectionStatus = "Disconnecting..."
        bluetoothGatt?.disconnect()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            permissionLauncher.launch(
                arrayOf(
                    Manifest.permission.BLUETOOTH_SCAN,
                    Manifest.permission.BLUETOOTH_CONNECT,
                    Manifest.permission.ACCESS_FINE_LOCATION
                )
            )
        } else {
            if (bluetoothAdapter?.isEnabled == false) {
                val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
                enableBluetoothLauncher.launch(enableBtIntent)
            }
        }

        enableEdgeToEdge()
        setContent {
            ScreenLockappTheme {
                appNavScreen(
                    connectionStatus = connectionStatus,
                    onSearchClicked = { startEspBluetoothSearch() }
                )
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        if (isScanning) stopBleScanSilently()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S &&
            ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED) {
            bluetoothGatt?.close()
        } else {
            bluetoothGatt?.close()
        }
    }
}