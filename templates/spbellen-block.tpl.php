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
    <div id="spbellen_completed" style="width:<?php print 0.98 * $stats['only_active']['completed_percentage']; ?>%"></div>
    <div id="spbellen_call_back" style="width:<?php print 0.98 * $stats['only_active']['call_back_percentage']; ?>%"></div>
    <div id="spbellen_not_contacted" style="width:<?php print 0.98 * $stats['only_active']['not_contacted_percentage']; ?>%"></div>
    <div id="spbellen_unreachable" style="width:<?php print 0.98 * $stats['only_active']['unreachable_percentage']; ?>%"></div>
    <div id="spbellen_wrong_number" style="width:<?php print 0.98 * $stats['only_active']['wrong_number_percentage']; ?>%"></div>
    <div id="spbellen_blacklisted" style="width:<?php print 0.98 * $stats['only_active']['blacklisted_percentage']; ?>%"></div>
    <div class='tooltip'>
      <table>
        <tr class='spbellen_completed'><td>groen</td><td>afgerond</td></tr>
        <tr class='spbellen_call_back'><td>geel</td><td>terugbellen</td></tr>
        <tr class='spbellen_not_contacted'><td>grijs</td><td>niet gebeld</td></tr>
        <tr class='spbellen_unreachable'><td>oranje</td><td>onbereikbaar</td></tr>
        <tr class='spbellen_wrong_number'><td>rood</td><td>verkeerd nummer</td></tr>
        <tr class='spbellen_blacklisted'><td>zwart</td><td>nooit meer bellen</td></tr>
      </table>
    </div>
  </div>
  <div class="wrapper left">
    <div class="content top">
      <h1><?php $phone = preg_replace("/[^0-9]/", "", $phone); ?><?php print substr($phone, 0, 2) . ' ' . chunk_split(substr($phone, 2), 4, ' '); ?></h1>
      <?php $gender_trans = array('Man' => 'm', 'Vrouw' => 'v', 'Anders' => '?'); ?>
      <h2><?php print $display_name; ?><?php print !empty($gender) ? ' ('.$gender_trans[$gender].')' : ''; ?></h2>
    </div>
    <div class="wrapper bottom">
      <?php if (!empty($email)) : ?>
        <div class="content left">
          <ul>
            <li><?php print !empty($email) ? $email : ''; ?></li>
          </ul>
        </div>
      <?php endif; ?>
      <?php if (!empty($street_address) || !empty($city)) : ?>
        <div class="content right">
          <ul><li><?php print !empty($street_address) ? $street_address : 'straat onbekend'; ?></li><li><?php print !empty($city) ? $city : 'stad onbekend'; ?></li></ul>
        </div>
      <?php endif; ?>
    </div>
  </div>
  <div class="content right">
    <p><strong>Belgeschiedenis</strong></p>
    <?php if (!empty($history)) : ?>
      <ul>
        <?php foreach ($history as $values) : ?>
          <li><?php print $values['date']; ?><br/><?php print $values['result']; ?><?php if (!empty($values['caller_message'])) : ?><br/>Opmerking: "<?php print $values['caller_message']; ?>"<?php endif; ?></li>
        <?php endforeach; ?>
      </ul>
    <?php else : ?>
        <p>Er is nog niet eerder gebeld</p>
    <?php endif; ?>
    <?php if (!empty($notes)) : ?>
      <p><strong>Notities</strong></p>
      <ul>
      <?php foreach ($notes as $note) : ?>
        <li><?php print $note; ?></li>
      <?php endforeach; ?>
      </ul>
    <?php endif; ?>
    <?php if (!empty($click2dial)) : ?>
      <div>
        <a id="spbellen-click2dial-button" href="#">Bel</a>
      </div>
    <?php endif; ?>
  </div>
</div>
