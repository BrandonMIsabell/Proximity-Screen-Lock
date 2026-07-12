package com.example.screenlockapp.ui.screens

// React to variable changes:
import androidx.compose.runtime.getValue
import androidx.compose.runtime.setValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember

// Columns
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.Alignment

// Text and Buttons
import androidx.compose.material3.Text
import androidx.compose.material3.Button
import androidx.compose.ui.unit.sp
import androidx.compose.ui.graphics.Color

import androidx.compose.runtime.Composable
@Composable
fun dashBoardScreen(connectionStatus: String, onSearchClicked: () -> Unit) {
    Column(
        modifier = Modifier.fillMaxSize(), // Makes the column take up the whole screen
        horizontalAlignment = Alignment.CenterHorizontally // Centers items left-to-right
    ) {
        // 1. A simple Text element displaying your State variable
        Text(
            text = "Status: $connectionStatus",
            fontSize = 20.sp,
            color = if ( connectionStatus == "Disconnected" ) Color.Red else Color.Cyan

        )
        Spacer(modifier = Modifier.height(32.dp))

        // 2. Connection search for ESP Button
        Button(
            onClick = onSearchClicked
        ) {
            // The visual contents of the button go here
            Text(text = "ESP Connection Search")
        }

    }
}