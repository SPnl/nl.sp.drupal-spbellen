Drupal.Nodejs.callbacks.spbellen_update_phone_number = {
  callback: function (message) {
  	//console.log(message);
  	if (message.callback == 'spbellen_update_phone_number') {
	    // Do whatever I want on the client's browser from here
      button = document.getElementById("spbellen-call-button");
      button.textContent = '----------';
      display_name = document.getElementById("spbellen-display-name");
      display_name.textContent = '                      ';
      setTimeout(function(){button.textContent = message.data.phone_number; button.href = 'tel:' + message.data.phone_number; display_name.textContent = message.data.display_name}, 500);
  	}
  }
};
