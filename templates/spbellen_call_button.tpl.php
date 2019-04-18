<div>
  <?php if (empty($phone_number)) : ?>
    <a id="spbellen-call-button"">Log op uw computer in</a>
  <?php else : ?>
    <a id="spbellen-call-button" href="tel:<?php print $phone_number; ?>"><?php print $phone_number; ?></a>
    <span id="spbellen-display-name"><?php print $display_name; ?></span>
  <?php endif; ?>
</div>
