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

<?php if(!empty($campaign_stats)) : ?>
  <h3><?php print($general_stats['title']); ?></h3>
  <table style="width: auto">
    <tr>
      <th></th><th>aantal</th><th>percentage</th>
    </tr>
    <tr>
      <td>Afgerond</td><td><?php print $general_stats['completed']; ?></td><td><?php print $general_stats['completed_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Terugbellen</td><td><?php print $general_stats['call_back']; ?></td><td><?php print $general_stats['call_back_percentage']; ?>%</td>

    </tr>
    <tr>
      <td>Niet gebeld</td><td><?php print $general_stats['not_contacted']; ?></td><td><?php print $general_stats['not_contacted_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Verkeerd nummer</td><td><?php print $general_stats['wrong_number']; ?></td><td><?php print $general_stats['wrong_number_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Nooit meer bellen</td><td><?php print $general_stats['blacklisted']; ?></td><td><?php print $general_stats['blacklisted_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Totaal</td><td><?php print $general_stats['total']; ?></td><td>100%</td>
    </tr>
  </table>
<?php endif; ?>
<?php if(!empty($campaign_stats)) : ?>
  <?php foreach($campaign_stats as $data) : ?>
    <h3><?php print ($data['title']); ?> (<a href="/node/<?php print $data['webform_id']; ?>/webform-results/analysis" target="_blank">resultaten)</a></h3>
    <table style="width: auto">
      <tr>
        <th></th><th colspan="2">Alle contacten</th><th colspan="2">Actieve contacten</th>
      </tr>
      <tr>
        <th></th><th>aantal</th><th>percentage</th><th>aantal</th><th>percentage</th>
      </tr>
      <tr>
        <td>Afgerond</td>
        <td><?php print $data['all']['completed']; ?></td>
        <td><?php print $data['all']['completed_percentage']; ?>%</td>
        <td><?php print $data['no_inactive']['completed']; ?></td>
        <td><?php print $data['no_inactive']['completed_percentage']; ?>%</td>
      </tr>
      <tr>
        <td>Terugbellen</td>
        <td><?php print $data['all']['call_back']; ?></td>
        <td><?php print $data['all']['call_back_percentage']; ?>%</td>
        <td><?php print $data['no_inactive']['call_back']; ?></td>
        <td><?php print $data['no_inactive']['call_back_percentage']; ?>%</td>

      </tr>
      <tr>
        <td>Niet gebeld</td>
        <td><?php print $data['all']['not_contacted']; ?></td>
        <td><?php print $data['all']['not_contacted_percentage']; ?>%</td>
        <td><?php print $data['no_inactive']['not_contacted']; ?></td>
        <td><?php print $data['no_inactive']['not_contacted_percentage']; ?>%</td>
      </tr>
      <tr>
        <td>Verkeerd nummer</td>
        <td><?php print $data['all']['wrong_number']; ?></td>
        <td><?php print $data['all']['wrong_number_percentage']; ?>%</td>
        <td><?php print $data['no_inactive']['wrong_number']; ?></td>
        <td><?php print $data['no_inactive']['wrong_number_percentage']; ?>%</td>
      </tr>
      <tr>
        <td>Nooit meer bellen</td>
        <td><?php print $data['all']['blacklisted']; ?></td>
        <td><?php print $data['all']['blacklisted_percentage']; ?>%</td>
        <td><?php print $data['no_inactive']['blacklisted']; ?></td>
        <td><?php print $data['no_inactive']['blacklisted_percentage']; ?>%</td>
      </tr>
      <tr>
        <td>Totaal</td>
        <td><?php print $data['all']['total']; ?></td>
        <td>100%</td>
        <td><?php print $data['no_inactive']['total']; ?></td>
        <td>100%</td>
      </tr>
    </table>
  <?php endforeach; ?>
<?php endif; ?>
