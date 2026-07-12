package com.example.screenlockapp.ui.navigation

import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.example.screenlockapp.ui.screens.*


@Composable
fun appNavScreen(
    connectionStatus: String,         // Accepts state from MainActivity
    onSearchClicked: () -> Unit       // Accepts execution callback from MainActivity
) {
    val navController = rememberNavController()

    NavHost(
        navController = navController,
        startDestination = "dashboard"
    ){
        //------- Login Screen --------
        composable("welcome") {
            Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                loginScreen(
                    name = "Android",
                    modifier = Modifier.padding(innerPadding)
                )
            }
        }

        // ------- Dashboard Screen --------
        composable("dashboard") {
            // Hand the centralized inputs off to the screen layout
            dashBoardScreen(
                connectionStatus = connectionStatus,
                onSearchClicked = onSearchClicked
            )
        }

        //------- App Selection Screen --------
        composable("app_selection") {
            appSelecScreen()
        }

        //------- Time Block Window Screen --------
        composable("time_block") {
            setTimeBlockScreen()
        }

        //------- Lock Screen --------
        composable("lock_screen") {
            lockAppScreen()
        }
    }
}