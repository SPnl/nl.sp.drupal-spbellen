(function ($) {

Drupal.behaviors.spBellenClick2Dial = {
  attach: function (context) {

   var click2dialUrl = Drupal.settings.spbellen.click2dial.script_url;
   var click2dialParams = Drupal.settings.spbellen.click2dial.script_params;
   var shortname = click2dialParams.shortname;
   var dest = click2dialParams.dest;
   var extension = click2dialParams.extension;
   $('#spbellen-click2dial-button').click(function( event ) {
     event.preventDefault();
     //console.log( click2dialUrl );
     //console.log( click2dialParams );
     //console.log( shortname );
     //console.log( dest );
     //console.log( extension );
     //var click2dial_result = $.get(click2dialUrl, click2dialParams);
     var click2dial_result = $.ajax({
       type: "GET",
       url: click2dialUrl,
       dataType: "html",
       data: click2dialParams,
       error: function(request, status, error) {
         //alert( 'Error: ' + request.responseText );
       },
       success: function(msg){ //alert( "Success: " + msg ); }
     }
     );

   });

  }
};

})(jQuery);
