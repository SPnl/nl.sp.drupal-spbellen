<?php
/*
 * Available variables:
 *   campaign_stats
 *     title
 *     total
 *     blacklisted
 *     call_back
 *     completed
 *     wrong_number
 */
?>


<?php foreach($campaign_stats as $data) : ?>
  <h3><?php print($data['title']); ?></h3>
  <table style="width: auto">
    <tr>
      <th></th><th>aantal</th><th>percentage</th>
    </tr>
    <tr>
      <td>Afgerond</td><td><?php print $data['completed']; ?></td><td><?php print $data['completed_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Terugbellen</td><td><?php print $data['call_back']; ?></td><td><?php print $data['call_back_percentage']; ?>%</td>

    </tr>
    <tr>
      <td>Niet gebeld</td><td><?php print $data['not_contacted']; ?></td><td><?php print $data['not_contacted_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Verkeerd nummer</td><td><?php print $data['wrong_number']; ?></td><td><?php print $data['wrong_number_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Nooit meer bellen</td><td><?php print $data['blacklisted']; ?></td><td><?php print $data['blacklisted_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Totaal</td><td><?php print $data['total']; ?></td><td>100%</td>
    </tr>
  </table>
<?php endforeach; ?>
