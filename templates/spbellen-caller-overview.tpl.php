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

<?php if(!empty($general_stats['caller'])) : ?>
  <h3>Algemene statistieken</h3>
  <?php $fields = array(
    'answered_completed' => 'Afgerond',
    'answered_call_back' => 'Terugbellen',
    'call_appointment' => 'Belafspraak',
    'skipped' => 'Overgeslagen',
    'wrong_number' => 'Verkeerd nummer',
    'blacklisted' => 'Nooit meer bellen',
    'empty' => '',
    'total' => 'Totaal (exclusief niet opgenomen)',
    'not_answered' => 'Niet opgenomen',
  ); ?>
  <?php $data_sets = array(
    'caller',
    'caller_campaign',
    'campaign',
    'caller_campaign_date',
    'campaign_date',
    'caller_date',
    'date',
  ); ?>
  <table style="width: auto">
    <tr>
      <th></th>
      <th colspan="3">beller&nbsp;totaal</th>
      <?php if (!empty($general_stats['caller_campaign'])) : ?>
        <th colspan="3">beller&nbsp;campagne</th>
      <?php endif; ?>
      <?php if (!empty($general_stats['campaign'])) : ?>
        <th colspan="3">campagne</th>
      <?php endif; ?>
      <?php if (!empty($general_stats['caller_campaign_date'])) : ?>
        <th colspan="3">beller&nbsp;campagne<br/><?php print($general_stats['caller_campaign_date']['call_date']); ?></th>
      <?php endif; ?>
      <?php if (!empty($general_stats['campaign_date'])) : ?>
        <th colspan="3">campagne<br/><?php print($general_stats['campaign_date']['call_date']); ?></th>
      <?php endif; ?>
      <?php if (!empty($general_stats['caller_date'])) : ?>
        <th colspan="3">beller<br/><?php print($general_stats['date']['call_date']); ?></th>
      <?php endif; ?>
      <?php if (!empty($general_stats['date'])) : ?>
        <th colspan="3">alle<br/><?php print($general_stats['date']['call_date']); ?></th>
      <?php endif; ?>
    </tr>
    <?php foreach ($fields as $field_key => $field_title): ?>
      <tr>
        <td width="200"><?php print $field_title; ?></td>
        <?php foreach ($data_sets as $data_set): ?>
          <?php if (!empty($general_stats[$data_set])) : ?>
            <td class="numeric" width="1"><?php print $general_stats[$data_set][$field_key]; ?></td>
            <td class="numeric" width="1"><?php print $general_stats[$data_set][$field_key . '_percentage']; ?><?php $percentage = $field_key == 'empty' ? '&nbsp;' : '%'; print $percentage; ?></td>
            <td></td>
          <?php endif; ?>
        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </table>
<?php endif; ?>

<?php if (!empty($result_stats_caller_campaign)) : ?>
  <p><br/></p>
  <h3>Resultaat statistieken</h3>
  <p>(Aantal - Percentage vragen afgerond - percentage telefoon opgenomen)</p>
  <?php foreach ($result_stats_caller_campaign['components'] as $cid => $data) : ?>
    <p><strong><?php print($data['name']); ?></strong></p>
    <table style="width: auto">
      <tr>
        <th></th>
        <th colspan="3">beller</th>
        <th colspan="3">campagne</th>
        <?php if (!empty($result_stats_caller_campaign_date)) : ?>
          <th colspan="3">beller<br/><?php print $result_stats_caller_campaign_date['call_date']; ?></th>
          <th colspan="3">campagne<br/><?php print $result_stats_caller_campaign_date['call_date']; ?></th>
        <?php endif; ?>
      </tr>
      <?php foreach ($data['options'] as $key => $option) : ?>
        <?php if ($key == 'answered_completed') : ?>
          <tr><td>&nbsp;</td><td></td><td></td><td></td><td></td><td></td><td></td>
          <?php if (!empty($result_stats_caller_campaign_date)) : ?>
            <td></td><td></td><td></td><td></td><td></td><td></td>
          <?php endif; ?>
          </tr>
        <?php endif; ?>
        <tr>
          <td width="200"><?php print($option); ?></td>
          <td class="numeric" width="1"><?php print($data['stats'][$key]); ?></td>
          <td class="numeric"><?php print($data['stats'][$key. '_percentage_answered_completed']); ?>%</td>
          <td class="numeric"><?php print($data['stats'][$key. '_percentage_answered']); ?>%</td>
          <td class="numeric" width="1"><?php print($result_stats_campaign['components'][$cid]['stats'][$key]); ?></td>
          <td class="numeric"><?php print($result_stats_campaign['components'][$cid]['stats'][$key . '_percentage_answered_completed']); ?>%</td>
          <td class="numeric"><?php print($result_stats_campaign['components'][$cid]['stats'][$key . '_percentage_answered']); ?>%</td>
          <?php if (!empty($result_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print($result_stats_caller_campaign_date['components'][$cid]['stats'][$key]); ?></td>
            <td class="numeric"><?php print($result_stats_caller_campaign_date['components'][$cid]['stats'][$key . '_percentage_answered_completed']); ?>%</td>
            <td class="numeric"><?php print($result_stats_caller_campaign_date['components'][$cid]['stats'][$key . '_percentage_answered']); ?>%</td>
            <td class="numeric" width="1"><?php print($result_stats_campaign_date['components'][$cid]['stats'][$key]); ?></td>
            <td class="numeric"><?php print($result_stats_campaign_date['components'][$cid]['stats'][$key . '_percentage_answered_completed']); ?>%</td>
            <td class="numeric"><?php print($result_stats_campaign_date['components'][$cid]['stats'][$key . '_percentage_answered']); ?>%</td>
          <?php endif; ?>
        </tr>
      <?php endforeach; ?>
    </table>
  <?php endforeach; ?>
<?php endif; ?>
