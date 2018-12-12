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

<?php if(!empty($general_stats_caller)) : ?>
  <h3><?php print($general_stats_caller_campaign['campaign_title']); ?></h3>
  <h3><?php print($general_stats_caller['caller_display_name']); ?></h3>
  <p><br/></p>
  <h3>Algemene statistieken</h3>
  <table style="width: auto">
    <tr>
      <th></th>
      <th colspan="2">beller&nbsp;totaal</th>
      <th colspan="2">beller&nbsp;campagne</th>
      <th colspan="2">campagne</th>
    </tr>
    <tr>
      <td width="200">Afgerond</td>
      <td width="1"><?php print $general_stats_caller['completed']; ?></td>
      <td><?php print $general_stats_caller['completed_percentage']; ?>%</td>
      <td width="1"><?php print $general_stats_caller_campaign['completed']; ?></td>
      <td><?php print $general_stats_caller_campaign['completed_percentage']; ?>%</td>
      <td width="1"><?php print $general_stats_campaign['completed']; ?></td>
      <td><?php print $general_stats_campaign['completed_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Terugbellen</td>
      <td><?php print $general_stats_caller['call_back']; ?></td>
      <td><?php print $general_stats_caller['call_back_percentage']; ?>%</td>
      <td><?php print $general_stats_caller_campaign['call_back']; ?></td>
      <td><?php print $general_stats_caller_campaign['call_back_percentage']; ?>%</td>
      <td><?php print $general_stats_campaign['call_back']; ?></td>
      <td><?php print $general_stats_campaign['call_back_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Verkeerd nummer</td>
      <td><?php print $general_stats_caller['wrong_number']; ?></td>
      <td><?php print $general_stats_caller['wrong_number_percentage']; ?>%</td>
      <td><?php print $general_stats_caller_campaign['wrong_number']; ?></td>
      <td><?php print $general_stats_caller_campaign['wrong_number_percentage']; ?>%</td>
      <td><?php print $general_stats_campaign['wrong_number']; ?></td>
      <td><?php print $general_stats_campaign['wrong_number_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Nooit meer bellen</td>
      <td><?php print $general_stats_caller['blacklisted']; ?></td>
      <td><?php print $general_stats_caller['blacklisted_percentage']; ?>%</td>
      <td><?php print $general_stats_caller_campaign['blacklisted']; ?></td>
      <td><?php print $general_stats_caller_campaign['blacklisted_percentage']; ?>%</td>
      <td><?php print $general_stats_campaign['blacklisted']; ?></td>
      <td><?php print $general_stats_campaign['blacklisted_percentage']; ?>%</td>
    </tr>
    <tr>
      <td>Totaal</td>
      <td><?php print $general_stats_caller['total']; ?></td>
      <td>100%</td>
      <td><?php print $general_stats_caller_campaign['total']; ?></td>
      <td><?php print $general_stats_caller_campaign['total_percentage']; ?>%</td>
      <td><?php print $general_stats_campaign['total']; ?></td>
      <td><?php print $general_stats_campaign['total_percentage']; ?>%</td>
    </tr>
  </table>
<?php endif; ?>

<?php if (!empty($result_stats_caller_campaign)) : ?>
  <p><br/></p>
  <h3>Resultaat statistieken</h3>
  <?php foreach ($result_stats_caller_campaign['components'] as $cid => $data) : ?>
    <p><strong><?php print($data['name']); ?></strong></p>
    <table style="width: auto">
      <tr>
        <th></th>
        <th colspan="2">beller</th>
        <th colspan="2">campagne</th>
      </tr>
      <?php foreach ($data['options'] as $key => $option) : ?>
        <tr>
          <td width="200"><?php print($option); ?></td>
          <td width="1"><?php print($data['stats'][$key]); ?></td>
          <td><?php print($data['stats'][$key. '_percentage']); ?>%</td>
          <td width="1"><?php print($result_stats_campaign['components'][$cid]['stats'][$key]); ?></td>
          <td><?php print($result_stats_campaign['components'][$cid]['stats'][$key . '_percentage']); ?>%</td>
        </tr>
      <?php endforeach; ?>
    </table>
  <?php endforeach; ?>
<?php endif; ?>
