(function ($) {

Drupal.behaviors.spBellenClick2Dial = {
  attach: function (context) {
   
   if (Drupal.settings.spbellen) {
     var click2dialUrl = Drupal.settings.spbellen.click2dial.script_url;
     var click2dialParams = Drupal.settings.spbellen.click2dial.script_params;
     var shortname = click2dialParams.shortname;
     var dest = click2dialParams.dest;
     var extension = click2dialParams.extension;
     $('#spbellen-click2dial-button').click(function( event ) {
       event.preventDefault();
       var click2dial_result = $.ajax({
         type: "GET",
         url: click2dialUrl,
         dataType: "html",
         data: click2dialParams,
       });
     });
   }

  }
};

})(jQuery);
