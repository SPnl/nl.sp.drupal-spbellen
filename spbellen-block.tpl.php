<?php
/*
 * Template for callers block.
 *
 * Available variables in function spbellen_contact_fields.
 *
 */
?>

<div id="spbellen-block-data">
  <div class="wrapper left">
    <div class="content top">
      <h1><?php print $phone; ?></h1>
      <?php $gender_trans = array('Man' => 'm', 'Vrouw' => 'v', 'Anders' => '?'); ?>
      <h2><?php print $display_name; ?> (<?php print $gender_trans[$gender]; ?>)</h2>
    </div>
    <div class="wrapper bottom">
      <div class="content left">
        <ul>
          <li><?php print $email; ?></li>
          <?php if (!empty($is_lid)) : ?>
            <li>Lid nr. <?php print $contact_id; ?></li>
          <?php else : ?>
            <li>Geen lid</li>
          <?php endif; ?>
        </ul>
      </div>
      <div class="content right">
        <ul><li><?php print $street_address; ?></li><li><?php print $city; ?></li></ul>
      </div>
    </div>
  </div>
  <?php if (!empty($history)) : ?>
    <div class="content right">
      <p><strong>Belgeschiedenis</strong></p>
      <ul>
        <?php foreach ($history as $values) : ?> 
          <li><?php print $values['date']; ?><br/><?php print $values['result']; ?></li>
        <?php endforeach; ?> 
      </ul>
    </div>
  <?php else : ?>
    <div class="content right">
      <p>Er is nog niet eerder gebeld</p>
    </div>
  <?php endif; ?>
</div>
