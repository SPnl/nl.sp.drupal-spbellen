(function ($) {

Drupal.behaviors.spBellenCampaignBlockOverlay = {
  attach: function (context) {

   var currentNid = Drupal.settings.spbellen.currentNid;
   var showOverlay = Drupal.settings.spbellen.showOverlay;

   // If we already closed the block: Do nothing.
   if ($.cookie("spbellen_campaign_intro_block_closed_" + currentNid)) {
     return;
   }
 
   if (showOverlay) {
     var block = $('#block-spbellen-spbellen-campaign-intro-block').first();
     block.addClass('block-spbellen-spbellen-campaign-intro-block-show');

     // Assign a click handler for the close button.
     $('#block-spbellen-spbellen-campaign-intro-block').click(function() {
       $('#block-spbellen-spbellen-campaign-intro-block').remove();
       $.cookie("spbellen_campaign_intro_block_closed_" + currentNid, true);
     });
   }
  }
};

})(jQuery);
