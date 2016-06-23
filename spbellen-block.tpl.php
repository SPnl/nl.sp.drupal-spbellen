<?php
/*
 * Template for callers block.
 *
 * Available variables in function spbellen_contact_fields.
 *
 */
?>

<div id="spbellen-block-data">
  <div id="spbellen-stats">
    <div id="spbellen_completed_percentage" style="width:<?php print $stats['completed_percentage']; ?>%"></div>
    <div id="spbellen_call_back_percentage" style="width:<?php print $stats['call_back_percentage']; ?>%"></div>
    <div id="spbellen_not_contacted_percentage" style="width:<?php print $stats['not_contacted_percentage']; ?>%"></div>
    <div id="spbellen_wrong_number_percentage" style="width:<?php print $stats['wrong_number_percentage']; ?>%"></div>
    <div id="spbellen_blacklisted_percentage" style="width:<?php print $stats['blacklisted_percentage']; ?>%"></div>
  </div>
  <div class="wrapper left">
    <div class="content top">
      <h1><?php print substr($phone, 0, 2) . ' ' . chunk_split(substr($phone, 2), 4, ' '); ?></h1>
      <?php $gender_trans = array('Man' => 'm', 'Vrouw' => 'v', 'Anders' => '?'); ?>
      <h2><?php print $display_name; ?><?php print !empty($gender) ? ' ('.$gender_trans[$gender].')' : ''; ?></h2>
    </div>
    <div class="wrapper bottom">
      <div class="content left">
        <ul>
          <li><?php print !empty($email) ? $email : ''; ?></li>
          <?php if (!empty($is_lid)) : ?>
            <li>Lid nr. <?php print $contact_id; ?></li>
          <?php else : ?>
            <li>Geen lid</li>
          <?php endif; ?>
        </ul>
      </div>
      <div class="content right">
        <ul><li><?php print !empty($street_address) ? $street_address : 'straat onbekend'; ?></li><li><?php print !empty($city) ? $city : 'stad onbekend'; ?></li></ul>
      </div>
    </div>
  </div>
  <?php if (!empty($history)) : ?>
    <div class="content right">
      <p><strong>Belgeschiedenis</strong></p>
      <ul>
        <?php foreach ($history as $values) : ?> 
        <li><?php print $values['date']; ?><br/><?php print $values['result']; ?><?php if (!empty($values['caller_message'])) : ?><br/>Opmerking: "<?php print $values['caller_message']; ?>"<?php endif; ?></li>
        <?php endforeach; ?> 
      </ul>
    </div>
  <?php else : ?>
    <div class="content right">
      <p>Er is nog niet eerder gebeld</p>
    </div>
  <?php endif; ?>
</div>
