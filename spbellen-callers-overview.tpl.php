<table style="width: auto">
  <tr>
    <th>Beller</th>
    <th colspan="2">Afgerond</th>
    <th colspan="2">Terugbellen</th>
    <th colspan="2">Belafspraak</th>
    <th colspan="2">Verkeer nummer</th>
    <th colspan="2">Nooit meer bellen</th>
    <th colspan="2">Totaal (excl. niet opgenomen)</th>
    <th colspan="2">Niet opgenomen</th>
  </tr>
  <?php foreach ($general_stats as $id => $caller_data) : ?>
    <?php if (is_numeric($id)) : ?>
      <tr>
        <td class="numeric"><?php print($caller_data['name']); ?></td>
        <td class="numeric"><?php print($caller_data['answered_completed']); ?></td>
        <td class="numeric"><?php print($caller_data['answered_completed_percentage']); ?>%</td>
        <td class="numeric"><?php print($caller_data['answered_call_back']); ?></td>
        <td class="numeric"><?php print($caller_data['answered_call_back_percentage']); ?>%</td>
        <td class="numeric"><?php print($caller_data['call_appointment']); ?></td>
        <td class="numeric"><?php print($caller_data['call_appointment_percentage']); ?>%</td>
        <td class="numeric"><?php print($caller_data['wrong_number']); ?></td>
        <td class="numeric"><?php print($caller_data['wrong_number_percentage']); ?>%</td>
        <td class="numeric"><?php print($caller_data['blacklisted']); ?></td>
        <td class="numeric"><?php print($caller_data['blacklisted_percentage']); ?>%</td>
        <td class="numeric"><?php print($caller_data['total']); ?></td>
        <td class="numeric"><?php print($caller_data['total_percentage']); ?>%</td>
        <td class="numeric"><?php print($caller_data['not_answered']); ?></td>
        <td class="numeric"><?php print($caller_data['not_answered_percentage']); ?>%</td>
      </tr>
    <?php endif; ?>
  <?php endforeach; ?>
</table>
