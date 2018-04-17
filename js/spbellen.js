(function ($) {

Drupal.behaviors.spBellen = {
  attach: function (context) {

    var isPreview = document.getElementsByClassName('webform-client-form preview');
    if (isPreview.length > 0) {
      $('body').addClass('preview');
      //console.log("Preview page!");  //here we can add our JS code
    }

    window.onbeforeunload = function (e) {
      e = e || window.event;

      // For Safari, other browsers use a default message
      return 'Weet je zeker dat je wilt afsluiten? Alle niet-opgeslagen gegevens zullen zijn verloren.';
    };
  }
};

})(jQuery);
