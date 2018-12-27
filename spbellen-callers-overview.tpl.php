<h3>Algemene statistieken</h3>
<table style="width: auto">
  <tr>
    <th>Beller</th>
    <th colspan="3">Afgerond</th>
    <th colspan="3">Terugbellen</th>
    <th colspan="3">Belafspraak</th>
    <th colspan="3">Overgeslagen</th>
    <th colspan="3">Verkeer nummer</th>
    <th colspan="3">Nooit meer bellen</th>
    <th colspan="3">Totaal (excl. niet opgenomen)</th>
    <th colspan="3">Niet opgenomen</th>
  </tr>
  <?php foreach ($general_stats as $id => $caller_data) : ?>
    <?php if (is_numeric($id)) : ?>
      <tr>
        <td><?php print($caller_data['name']); ?></td>
        <td class="numeric" width="1"><?php print($caller_data['answered_completed']); ?></td>
        <td class="numeric" width="1"><?php print($caller_data['answered_completed_percentage']); ?>%</td>
        <td></td>
        <td class="numeric" width="1"><?php print($caller_data['answered_call_back']); ?></td>
        <td class="numeric" width="1"><?php print($caller_data['answered_call_back_percentage']); ?>%</td>
        <td></td>
        <td class="numeric" width="1"><?php print($caller_data['call_appointment']); ?></td>
        <td class="numeric" width="1"><?php print($caller_data['call_appointment_percentage']); ?>%</td>
        <td></td>
        <td class="numeric" width="1"><?php print($caller_data['skipped']); ?></td>
        <td class="numeric" width="1"><?php print($caller_data['skipped_percentage']); ?>%</td>
        <td></td>
        <td class="numeric" width="1"><?php print($caller_data['wrong_number']); ?></td>
        <td class="numeric" width="1"><?php print($caller_data['wrong_number_percentage']); ?>%</td>
        <td></td>
        <td class="numeric" width="1"><?php print($caller_data['blacklisted']); ?></td>
        <td class="numeric" width="1"><?php print($caller_data['blacklisted_percentage']); ?>%</td>
        <td></td>
        <td class="numeric" width="1"><?php print($caller_data['total']); ?></td>
        <td class="numeric" width="1"><?php print($caller_data['total_percentage']); ?>%</td>
        <td></td>
        <td class="numeric" width="1"><?php print($caller_data['not_answered']); ?></td>
        <td class="numeric" width="1"><?php print($caller_data['not_answered_percentage']); ?>%</td>
        <td></td>
      </tr>
    <?php endif; ?>
  <?php endforeach; ?>
</table>

<h3>Resultaat statistieken</h3>
<?php foreach ($result_stats['components'] as $component_id => $component_data) : ?>
  <p><strong><?php print $component_data['name']; ?></strong></p>
  <table style = "width: auto">
    <tr>
      <th>Beller</th>
      <?php foreach ($component_data['options'] as $key => $value) : ?>
        <th colspan="3"><?php print $value; ?></th>
      <?php endforeach; ?>
    </tr>
    <?php foreach ($component_data['stats'] as $caller_id => $caller_data) : ?>
      <tr>
        <td><?php print $caller_data['display_name']; ?></td>
        <?php foreach ($component_data['options'] as $key => $value) : ?>
          <td class="numeric">
            <?php print $caller_data[$key]; ?>
          </td>
          <td class="numeric">
            <?php print $caller_data[$key . '_percentage']; ?>%
          </td>
          <td></td>
        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </table>
<?php endforeach; ?>
