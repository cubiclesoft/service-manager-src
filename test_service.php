<?php
	// A simple test service that does nothing.
	// (C) 2016 CubicleSoft.  All Rights Reserved.

	if (!isset($_SERVER["argc"]) || !$_SERVER["argc"])
	{
		echo "This file is intended to be run from the command-line.";

		exit();
	}

	// Temporary root.
	$rootpath = str_replace("\\", "/", dirname(__FILE__));

	function DisplayError($msg, $result = false, $exit = true)
	{
		ob_start();

		echo ($exit ? "[Error] " : "") . $msg . "\n";

		if ($result !== false)
		{
			if (isset($result["error"]))  echo "[Error] " . $result["error"] . " (" . $result["errorcode"] . ")\n";
			if (isset($result["info"]))  var_dump($result["info"]);
		}

		fwrite(STDERR, ob_get_contents());
		ob_end_clean();

		if ($exit)  exit();
	}

	if ($argc > 1)
	{
		// Helper class.  Technically, none of these options nor the SDK are necessary - they just make life a little easier.
		require_once $rootpath . "/sdks/servicemanager.php";

		$sm = new ServiceManager($rootpath);
//		$sm = new ServiceManager($rootpath . "/servicemanager/Debug");

		echo "Service manager:  " . $sm->GetServiceManagerRealpath() . "\n\n";

		$servicename = "servicemanager-php-test";

		if ($argv[1] == "install")
		{
			// Install the service.
			$args = array();
			$options = array();

			$result = $sm->Install($servicename, __FILE__, $args, $options, true);
			if (!$result["success"])  DisplayError("Unable to install the '" . $servicename . "' service.", $result);
		}
		else if ($argv[1] == "uninstall")
		{
			// Uninstall the service.
			$result = $sm->Uninstall($servicename, true);
			if (!$result["success"])  DisplayError("Unable to uninstall the '" . $servicename . "' service.", $result);
		}
		else if ($argv[1] == "dumpconfig")
		{
			$result = $sm->GetConfig($servicename);
			if (!$result["success"])  DisplayError("Unable to retrieve the configuration for the '" . $servicename . "' service.", $result);

			echo "Service configuration:  " . $result["filename"] . "\n\n";

			echo "Current service configuration:\n\n";
			foreach ($result["options"] as $key => $val)  echo "  " . $key . " = " . $val . "\n";
		}
		else
		{
			echo "Command not recognized.  Run the service manager directly for anything other than 'install', 'uninstall', and 'dumpconfig'.\n";
		}
	}
	else
	{
		// Main service code.
		$stopfilename = __FILE__ . ".notify.stop";
		$reloadfilename = __FILE__ . ".notify.reload";
		$lastservicecheck = time();
		$running = true;

		do
		{
			// Do regular work here.
			sleep(1);

			// Check the status of the two service file options.
			if ($lastservicecheck <= time() - 3)
			{
				if (file_exists($stopfilename))
				{
					// Initialize termination.
					echo "Stop requested.\n";

					$running = false;
				}
				else if (file_exists($reloadfilename))
				{
					// Reload configuration and then remove reload file.
					echo "Reload config requested.\n";

					@unlink($reloadfilename);
				}

				$lastservicecheck = time();
			}
		} while ($running);
	}
?>