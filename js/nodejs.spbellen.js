Drupal.Nodejs.callbacks.spbellen_update_phone_number = {
  callback: function (message) {
  	//console.log(message);
  	if (message.callback == 'spbellen_update_phone_number') {
	    // Do whatever I want on the client's browser from here
      button = document.getElementById("spbellen-call-button");
      button.textContent = '----------';
      setTimeout(function(){button.textContent = message.data.phone_number; button.href = 'tel:' + message.data.phone_number}, 500);
  	}
  }
};
