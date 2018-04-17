(function ($) {

Drupal.behaviors.spBellen = {
  attach: function (context) {

    var isPreview = document.getElementsByClassName('webform-client-form preview');
    if (isPreview.length > 0) {
      $('body').addClass('preview');
      //console.log("Preview page!");  //here we can add our JS code
    }

  }
};

})(jQuery);
