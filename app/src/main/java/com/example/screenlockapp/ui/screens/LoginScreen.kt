package com.example.screenlockapp.ui.screens

import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier

@Composable
fun loginScreen(name: String, modifier: Modifier = Modifier){
    androidx.compose.material3.Text(
        text = "Welcome to Proximity Lock $name",
        modifier = modifier
    )
}